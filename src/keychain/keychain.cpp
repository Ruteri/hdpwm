/*

Copyright (C) 2019 Mateusz Morusiewicz

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.

*/

#include <src/keychain/keychain.h>

#include <src/keychain/db.h>

#include <external/nlohmann/json_single_include.h>
using json = nlohmann::json;

#include <leveldb/db.h>

#include <list>
#include <stdexcept>

#include <fstream>
#include <iostream>

namespace keychain {

constexpr char DB_KEY_SEED[] = "seed";
constexpr char DB_KEY_DPATH[] = "dpath";
constexpr char DB_KEY_ENTRIES[] = "entries";

Keychain::Keychain(Keychain &&other) {
	this->data_path = std::move(other.data_path);
	this->db = std::move(other.db);
	other.db = nullptr;
	this->tec = std::move(other.tec);
}

Keychain &Keychain::operator=(Keychain &&other) {
	this->data_path = std::move(other.data_path);
	this->db = std::move(other.db);
	other.db = nullptr;
	this->tec = std::move(other.tec);
	return *this;
}

std::string get_default_db_layout() {
	const json empty_db =
	    json::parse(R"({ "name": "/", "details": "", "dirs": [], "entries": [] })");
	return empty_db.dump();
}

std::unique_ptr<Keychain> Keychain::initialize_with_seed(
    std::filesystem::path path, crypto::Seed &&seed, crypto::PasswordHash &&pw_hash) {
	std::unique_ptr<Keychain> kc = std::make_unique<Keychain>();
	kc->data_path = std::move(path);
	kc->tec = crypto::TimedEncryptionKey(std::move(pw_hash));

	leveldb::Options options;
	options.create_if_missing = true;

	if (!std::filesystem::create_directory(kc->data_path)) {
		throw std::runtime_error("could not create directory at given path");
	}

	auto db_path = kc->data_path / "db";
	kc->db = DB::Open(options, db_path.string());
	if (!kc->db) {
		throw std::runtime_error("could not initialize db");
	}

	crypto::EncryptedSeed encrypted_seed;
	kc->tec.encrypt(encrypted_seed.data(), seed.data(), crypto::Seed::Size);

	if (auto s = kc->db->Put(leveldb::WriteOptions(), DB_KEY_SEED,
	        crypto::serialize<crypto::EncryptedSeed>(encrypted_seed));
	    !s.ok()) {
		throw std::runtime_error("could not save seed in the database");
	}

	if (auto s = kc->db->Put(leveldb::WriteOptions(), DB_KEY_ENTRIES, get_default_db_layout());
	    !s.ok()) {
		throw std::runtime_error("could not save default layout in the database");
	}

	if (auto s = kc->db->Put(leveldb::WriteOptions(), DB_KEY_DPATH, std::to_string(0)); !s.ok()) {
		throw std::runtime_error("could not save default layout in the database");
	}

	return kc;
}

std::unique_ptr<Keychain> Keychain::open(std::filesystem::path path, crypto::PasswordHash pw_hash) {
	std::unique_ptr<Keychain> kc = std::make_unique<Keychain>();
	kc->data_path = path.string();
	kc->tec = crypto::TimedEncryptionKey(std::move(pw_hash));

	auto db_path = kc->data_path / "db";
	kc->db = DB::Open(leveldb::Options(), db_path.string());
	if (!kc->db) {
		throw std::runtime_error("could not open db");
	}

	return kc;
}

void Keychain::export_to_uri(const UriLocator &uri) const {
	std::string db_entries;
	if (auto s = db->Get(leveldb::ReadOptions(), DB_KEY_ENTRIES, &db_entries); !s.ok()) {
		throw std::runtime_error("could not get entries from db");
	}

	crypto::DerivationPath standard_encryption_path = {
	    255}; // TODO: should be predefined and reserved m/0/0
	crypto::Seed standard_encryption_seed = derive_child(standard_encryption_path);
	crypto::EncryptionKey key(standard_encryption_seed);
	auto encrypted_entries = crypto::encrypt(key, db_entries);
	if (auto path = std::get_if<std::filesystem::path>(&uri)) {
		std::ofstream export_file;
		export_file.open(*path, std::ios::out);
		std::ostream_iterator<uint8_t> output_iterator(export_file, "");
		std::copy(encrypted_entries.begin(), encrypted_entries.end(), output_iterator);
		export_file.close();
	} else {
		throw std::runtime_error("unexpected uri type");
	}
}

Directory::ptr Keychain::get_root_dir() const {
	std::string db_entries;
	if (auto s = db->Get(leveldb::ReadOptions(), DB_KEY_ENTRIES, &db_entries); !s.ok()) {
		throw std::runtime_error("could not get entries from db");
	}

	auto root = deserialize_directory(json::parse(db_entries), nullptr);
	root->is_open = true;
	return root;
}

// TODO: make it thread-safe if needed
crypto::DerivationPath Keychain::get_next_derivation_path() {
	std::string c_dpath_str{};
	if (auto s = db->Get(leveldb::ReadOptions(), DB_KEY_DPATH, &c_dpath_str); !s.ok()) {
		throw std::runtime_error("could not load seed from db");
	}

	int current_seed = std::stoi(c_dpath_str);
	++current_seed;

	if (auto s = db->Put(leveldb::WriteOptions(), DB_KEY_DPATH, std::to_string(current_seed));
	    !s.ok()) {
		throw std::runtime_error("could not save entries");
	}

	return {current_seed};
}

void Keychain::save_entries(Directory::ptr root) {
	json new_entries = serialize_directory(root);
	if (auto s = db->Put(leveldb::WriteOptions(), DB_KEY_ENTRIES, new_entries.dump()); !s.ok()) {
		throw std::runtime_error("could not save entries");
	}
}

constexpr static char allowed_chars[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/*()_-=&^%$#@!~}{|L?><M\\/.,><";

utils::sensitive_string Keychain::encode_secret(
    unsigned char *in_data, size_t in_size, size_t out_size) {
	utils::sensitive_string secret(out_size);
	size_t v{0x603d0ba6};
	for (size_t i = 0; i * 3 < in_size && i < out_size; ++i) {
		v += 0xcc455073;
		v *= static_cast<unsigned int>(in_data[i]) + 6416;
		v += static_cast<unsigned int>(in_data[i + 1]);
		v ^= static_cast<unsigned int>(in_data[i + 2]);
		v %= 0x6644e8e0;
		secret.push_back(allowed_chars[v % sizeof(allowed_chars)]);
	}

	return secret;
}

crypto::Seed Keychain::derive_child(const crypto::DerivationPath &dpath) const {
	std::string seed_str{};
	seed_str.reserve(crypto::Seed::Size + 1); // reserve to avoid leaving seed in memory
	if (auto s = db->Get(leveldb::ReadOptions(), DB_KEY_SEED, &seed_str); !s.ok()) {
		throw std::runtime_error("could not load seed from db");
	}

	crypto::EncryptedSeed encrypted_seed = crypto::deserialize<crypto::EncryptedSeed>(seed_str);
	utils::secure_zero_string(std::move(seed_str));

	crypto::Seed derived_seed =
	    crypto::derive_child(this->tec.getPasswordHash(), encrypted_seed, dpath);

	return derived_seed;
}

utils::sensitive_string Keychain::derive_secret(const crypto::DerivationPath &dpath) {
	crypto::Seed derived_seed = derive_child(dpath);
	return Keychain::encode_secret(derived_seed.data(), derived_seed.size(), 10);
}

} // namespace keychain
