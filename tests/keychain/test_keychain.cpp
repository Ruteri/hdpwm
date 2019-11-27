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

#include <src/keychain/db.h>

#include <leveldb/db.h>

#include <external/nlohmann/json_single_include.h>
using json = nlohmann::json;

#include <external/catch2/catch.hpp>

#include <fstream>
#include <cstdio>

class KeychainMock: public keychain::Keychain {
public:
	void set_db(std::unique_ptr<keychain::DB> db) { this->db = std::move(db); }
	void set_ec(crypto::PasswordHash pw_hash) { this->tec = crypto::TimedEncryptionKey(pw_hash); }

};

class DBMock: public keychain::DB {
public:
	~DBMock() final {}

	int Get_call_count = 0;
	std::vector<std::tuple<leveldb::ReadOptions, leveldb::Slice, std::string*>> Get_calls;
	std::function<leveldb::Status(const leveldb::ReadOptions&, const leveldb::Slice&, std::string*)> Get_mock_fn;
	leveldb::Status Get(const leveldb::ReadOptions& options, const leveldb::Slice& key, std::string* value) final {
		++Get_call_count;
		Get_calls.emplace_back( std::make_tuple(options, key, value) );
		return Get_mock_fn(options, key, value);
	}

	int Put_call_count = 0;
	std::vector<std::tuple<leveldb::WriteOptions, std::pair<leveldb::Slice, leveldb::Slice>>> Put_calls;
	std::function<leveldb::Status(const leveldb::WriteOptions&, const leveldb::Slice&, const leveldb::Slice&)> Put_mock_fn =
	    [](const leveldb::WriteOptions&, const leveldb::Slice&, const leveldb::Slice&) { return leveldb::Status(); };
	leveldb::Status Put(const leveldb::WriteOptions& options, const leveldb::Slice& key, const leveldb::Slice& value) final {
		++Put_call_count;
		Put_calls.push_back({ options, {key, value} });
		return Put_mock_fn(options, key, value);
	}
};

json sample_entries = json::parse(R"({ "name": "dir1", "details": "details1", "dirs": [{"name": "dir2", "details": "details2", "dirs": [], "entries": [{"name": "entry1", "details": "entry_details1", "derivation_path": 6}]}], "entries": [{"name": "entry2", "details": "entry_details2", "derivation_path": 7}] })");

crypto::PasswordHash sample_password_hash = crypto::deserialize<crypto::PasswordHash>("5e884898da28047151d0e56f8dc6292773603d0d6aabbdd62a11ef721d1542d8");

std::string sample_seed = "a0727f73ff7cb6eea580b5e808b26e28110add5a34481a7e2ac282e649c7d6feccf870a9448b901087adc0a224059e2855fcfe221c5db00dc598aad29c2593f6";

struct seed_data_s {
	std::string seed;
	std::string expected;
	int size;
};

TEST_CASE( "secret encoding works as intended", "[keychain_secret_encoding]" ) {

	std::vector<seed_data_s> data = {
		{ "cb167a75be85dda7988b628dd9e5ffbe13d3acd86fcf4cc0e742de17d80b3e25fb9781acc11821301f15bb2728225093c9b302dca2e05239785952218c3da735", R"(MkAsM%uZXu)", 10 },
		{ "cb167a75be85dda7988b628dd9e5ffbe13d3acd86fcf4cc0e742de17d80b3e25fb9781acc11821301f15bb2728225093c9b302dca2e05239785952218c3da735", R"(MkAsM%uZXu*HKQTQj.g8~)", 21 },
		{ "cb167a75be85dda7988b628dd9e5ffbe13d3acd86fcf4cc0e742de17d80b3e25fb9781acc11821301f15bb2728225093c9b302dca2e05239785952218c3da735", R"(MkAsM%uZXu*HKQTQj.g8~b)", 22 },
		{ "cb167a75be85dda7988b628dd9e5ffbe13d3acd86fcf4cc0e742de17d80b3e25fb9781acc11821301f15bb2728225093c9b302dca2e05239785952218c3da735", R"(MkAsM%uZXu*HKQTQj.g8~b)", 23 },
	};

	for (auto &tc : data) {
		auto seed = crypto::deserialize<crypto::Seed>(tc.seed);
		auto encoded = keychain::Keychain::encode_secret(seed.data(), seed.size(), tc.size);
		REQUIRE( static_cast<std::string>(encoded) == tc.expected );
	}
}

TEST_CASE( "secrets are derived properly", "[keychain_derive_secret]" ) {
	auto db = new DBMock();

	db->Get_mock_fn = [](const leveldb::ReadOptions&, const leveldb::Slice& key, std::string* value) {
		REQUIRE( key.ToString() == "seed" );
		*value = "a0727f73ff7cb6eea580b5e808b26e28110add5a34481a7e2ac282e649c7d6feccf870a9448b901087adc0a224059e2855fcfe221c5db00dc598aad29c2593f6";
		return leveldb::Status();
	};

	KeychainMock kc;
	kc.set_db(std::unique_ptr<keychain::DB>(db));
	kc.set_ec(sample_password_hash);

	auto rv = kc.derive_secret({ 1 });

	REQUIRE( static_cast<std::string>(rv) == "MkAsM%uZXu" );
}

TEST_CASE( "entries are saved as expected", "[keychain_save_entries]" ) {
	auto db = new DBMock();

	KeychainMock kc;
	kc.set_db(std::unique_ptr<keychain::DB>(db));

	auto root = keychain::deserialize_directory(sample_entries, 0);

	kc.save_entries(root);

	REQUIRE( db->Get_call_count == 0 );
	REQUIRE( db->Put_call_count == 1 );
	REQUIRE( std::get<1>(db->Put_calls[0]) == std::pair<leveldb::Slice, leveldb::Slice>{"entries", sample_entries.dump()} );
}

TEST_CASE( "entries' root is created properly", "[keychain_get_root]" ) {
	auto db = new DBMock();

	db->Get_mock_fn = [](const leveldb::ReadOptions&, const leveldb::Slice& key, std::string* value) {
		REQUIRE( key.ToString() == "entries" );
		*value = sample_entries.dump();
		return leveldb::Status();
	};

	KeychainMock kc;
	kc.set_db(std::unique_ptr<keychain::DB>(db));

	auto root = kc.get_root_dir();

	/* do basic checks, deserialization is covered by test_keychain_entry.cpp */
	REQUIRE( root->meta.name == "dir1" );
	REQUIRE( root->dirs.size() == 1 );
	REQUIRE( root->entries.size() == 1 );

	REQUIRE( root->dirs[0]->meta.name == "dir2" );
	REQUIRE( root->dirs[0]->dirs.size() == 0 );
	REQUIRE( root->dirs[0]->entries.size() == 1 );
	REQUIRE( root->dirs[0]->entries[0]->meta.name == "entry1" );

	REQUIRE( root->entries[0]->meta.name == "entry2" );
}

TEST_CASE( "can export and import", "[keychain_export_import]" ) {
	auto db = new DBMock();

	db->Get_mock_fn = [](const leveldb::ReadOptions&, const leveldb::Slice& key, std::string* value) {
		if (key.ToString() == "entries") {
			*value = sample_entries.dump();
			return leveldb::Status();
		} else if (key.ToString() == "seed") {
			*value = sample_seed;
			return leveldb::Status();
		}

		REQUIRE( false );
		return leveldb::Status();
	};

	std::function<void(std::string*)> on_finish_delete = [](std::string* path) { std::remove(path->c_str()); delete path; };
	std::unique_ptr<std::string, decltype(on_finish_delete)> tmp_path (new std::string(std::tmpnam(nullptr)), on_finish_delete);

	{ /* export */
		KeychainMock kc;
		auto m_db = new DBMock(*db);
		kc.set_db(std::unique_ptr<keychain::DB>(m_db));
		kc.set_ec(sample_password_hash);

		kc.export_to_uri(*tmp_path);

		std::ifstream export_file(*tmp_path, std::ios::in);
		std::string fc;
		export_file >> fc;
		export_file.close();

		REQUIRE( fc == "T/CjXuX2Qa3cGUMS8PFQhUL7Jky7Dc+Wq19k/zCEy8HnymriHKnLed1FcDjS+PTR4m4dlZXLCORJBY40UKvcr7AxD+kZ2CH6N6LlM5TxLiTRBuXIA2lgj3rfOz3pBbqbf5qQUIflPUIVc1WgTtKBwzFot4BGn2EtpYIhox4utGGcPKTD00xmCL6I9ltdWoDRWw6bhPcM7evAgAJ5R5zyCeNATbAqmrLedNWd6Olrsiawi4cNESMeuWp9zkvnwhZf5Rk4awZ6KRHg/KGuLaP9sp3o/kp21VQwe5M+8v16eJFODwK75v4J7nEDB1qa6sVWNWQEXkWnYKMk15HZwbQj/plra76K6YMtKpSdZJ5YNC1m4VWjkZM0lWtFGuNH1VwV1DLjkcNCHJU0BTHL0p3huKOqC0b/JhMaRI8XwUHDKmce3UnSUMyw3A==" );
	}

	{ /* import */
		KeychainMock kc;
		auto m_db = new DBMock(*db);
		kc.set_db(std::unique_ptr<keychain::DB>(m_db));
		kc.set_ec(sample_password_hash);

		kc.import_from_uri(*tmp_path);
		REQUIRE( keychain::serialize_directory(kc.get_root_dir()) == sample_entries );
	}
}
