#include <src/keychain/keychain.h>

#include <external/nlohmann/json_single_include.h>
#include <leveldb/db.h>
using json = nlohmann::json;

#include <list>
#include <stdexcept>

constexpr char DB_KEY_SEED[] = "seed";
constexpr char DB_KEY_DPATH[] = "dpath";
constexpr char DB_KEY_ENTRIES[] = "entries";

Keychain::Keychain(Keychain &&other) {
	this->data_path = std::move(other.data_path);
	this->db = other.db;
	other.db = nullptr;
	this->tec = std::move(other.tec);
}

Keychain &Keychain::operator=(Keychain &&other) {
	this->data_path = std::move(other.data_path);
	this->db = other.db;
	other.db = nullptr;
	this->tec = std::move(other.tec);
	return *this;
}

Keychain::~Keychain() {
	if (this->db) delete this->db;
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
	leveldb::Status status = leveldb::DB::Open(options, db_path.string(), &kc->db);
	if (!status.ok()) {
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
	leveldb::Status status = leveldb::DB::Open(leveldb::Options(), db_path.string(), &kc->db);
	if (!status.ok()) {
		throw std::runtime_error("could not open db");
	}

	return kc;
}

namespace {

KeychainEntry::ptr deserialize_entry(const json &data, std::weak_ptr<KeychainDirectory> parent) {
	KeychainEntryMeta meta{
	    data["name"].get<std::string>(),
	    data["details"].get<std::string>(),
	    {
	        data["derivation_path"].get<int>(),
	    },
	};

	return std::make_shared<KeychainEntry>(meta, parent);
}

KeychainDirectory::ptr deserialize_directory(const json &data, int dir_level) {
	KeychainDirectoryMeta meta{
	    data["name"].get<std::string>(),
	    data["details"].get<std::string>(),
	};

	auto dir = std::make_shared<KeychainDirectory>(meta, dir_level);

	for (const json &entry : data["entries"]) {
		dir->entries.push_back(deserialize_entry(entry, dir));
	}

	for (const json &child_dir : data["dirs"]) {
		dir->dirs.push_back(deserialize_directory(child_dir, dir_level + 1));
	}

	return dir;
}

json serialize_entry(KeychainEntry::ptr entry) {
	return {{"name", entry->meta.name}, {"details", entry->meta.details},
	    {"derivation_path", entry->meta.dpath.seed}};
}

json serialize_directory(KeychainDirectory::ptr dir) {
	json entries = json::array();
	for (const auto &entry : dir->entries) {
		entries.push_back(serialize_entry(entry));
	}

	json dirs = json::array();
	for (const auto &child_dir : dir->dirs) {
		dirs.push_back(serialize_directory(child_dir));
	}

	return {{"name", dir->meta.name}, {"details", dir->meta.details}, {"dirs", std::move(dirs)},
	    {"entries", std::move(entries)}};
}

void process_flatten_dir(std::list<AnyKeychainPtr> *to_visit, KeychainDirectory::ptr dir) {
	if (!dir->is_open) {
		return;
	}
	for (int i = dir->entries.size() - 1; i >= 0; --i) {
		to_visit->push_front(dir->entries[i]);
	}
	for (int i = dir->dirs.size() - 1; i >= 0; --i) {
		to_visit->push_front(dir->dirs[i]);
	}
}

} // namespace

std::vector<AnyKeychainPtr> Keychain::flatten_dirs(KeychainDirectory::ptr root) {
	std::vector<AnyKeychainPtr> rv;
	std::list<AnyKeychainPtr> to_visit{root};

	while (!to_visit.empty()) {
		auto c_node_v = to_visit.front();
		to_visit.pop_front();

		rv.push_back(c_node_v);

		std::visit(
		    overloaded{[](KeychainEntry::ptr) {},
		        [&to_visit](KeychainDirectory::ptr dir) { process_flatten_dir(&to_visit, dir); }},
		    c_node_v);
	}

	return rv;
}

KeychainDirectory::ptr Keychain::get_root_dir() const {
	std::string db_entries;
	if (auto s = db->Get(leveldb::ReadOptions(), DB_KEY_ENTRIES, &db_entries); !s.ok()) {
		throw std::runtime_error("could not get entries from db");
	}

	auto root = deserialize_directory(json::parse(db_entries), 0);
	root->is_open = true;
	return root;
}

// TODO: not thread-safe
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

void Keychain::save_entries(KeychainDirectory::ptr root) {
	json new_entries = serialize_directory(root);
	if (auto s = db->Put(leveldb::WriteOptions(), DB_KEY_ENTRIES, new_entries.dump()); !s.ok()) {
		throw std::runtime_error("could not save entries");
	}
}

constexpr static char allowed_chars[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/*()_-=&^%$#@!~}{|L?><M\\/.,><";

utils::sensitive_string Keychain::encode_secret(
    unsigned char *in_data, size_t in_size, size_t out_size) {
	utils::sensitive_string secret;
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

utils::sensitive_string Keychain::derive_secret(const crypto::DerivationPath &dpath) {
	std::string seed_str{};
	seed_str.reserve(crypto::Seed::Size + 1); // reserve to avoid leaving seed in memory
	if (auto s = db->Get(leveldb::ReadOptions(), DB_KEY_SEED, &seed_str); !s.ok()) {
		throw std::runtime_error("could not load seed from db");
	}

	crypto::EncryptedSeed encrypted_seed = crypto::deserialize<crypto::EncryptedSeed>(seed_str);
	utils::secure_zero_string(seed_str);

	crypto::Seed derived_seed =
	    crypto::derive_child(this->tec.getPasswordHash(), encrypted_seed, dpath);

	return Keychain::encode_secret(derived_seed.data(), derived_seed.size(), 10);
}
