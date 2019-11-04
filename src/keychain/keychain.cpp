#include <src/keychain/keychain.h>

#include <leveldb/db.h>

#include <list>
#include <stdexcept>

constexpr char DB_KEY_SEED[] = "seed";

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
	kc->db->Put(leveldb::WriteOptions(), DB_KEY_SEED, encrypted_seed.serialize_to_string());

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

void process_flatten_dir(std::list<std::variant<KeychainDirectory *, KeychainEntry *>> &to_visit,
    KeychainDirectory *dir) {
	if (!dir->is_open) {
		return;
	}
	for (int i = dir->entries.size() - 1; i >= 0; --i) {
		to_visit.push_front(&(dir->entries[i]));
	}
	for (int i = dir->dirs.size() - 1; i >= 0; --i) {
		to_visit.push_front(&(dir->dirs[i]));
	}
}

} // namespace

std::vector<std::variant<KeychainDirectory *, KeychainEntry *>> flatten_dirs(
    std::shared_ptr<KeychainDirectory> root) {
	std::vector<std::variant<KeychainDirectory *, KeychainEntry *>> rv;
	std::list<std::variant<KeychainDirectory *, KeychainEntry *>> to_visit;

	process_flatten_dir(to_visit, root.get());

	while (!to_visit.empty()) {
		std::variant<KeychainDirectory *, KeychainEntry *> c_node_v = to_visit.front();
		to_visit.pop_front();

		rv.push_back(c_node_v);

		std::visit(overloaded{[](KeychainEntry *) {},
		               [&to_visit](KeychainDirectory *dir) { process_flatten_dir(to_visit, dir); }},
		    c_node_v);
	}

	return rv;
}
