#include <src/keychain/keychain.h>

#include <stdexcept>

constexpr char DB_KEY_SEED[] = "seed";

Keychain Keychain::initialize_with_seed(std::filesystem::path path, crypto::Seed&& seed, crypto::PasswordHash&& pw_hash) {
	Keychain kc;
	kc.tec = std::move(crypto::TimedEncryptionKey(std::move(pw_hash)));

	leveldb::Options options;
	options.create_if_missing = true;

	if (!std::filesystem::create_directory(path)) {
		throw std::runtime_error("could not create directory at given path");
	}

	auto db_path = path / "db";
	leveldb::Status status = leveldb::DB::Open(options, db_path.string(), &kc.db);
	if (!status.ok()) {
		throw std::runtime_error("could not initialize db");
	}

	crypto::EncryptedSeed encrypted_seed;
	kc.tec.encrypt(encrypted_seed.data(), seed.data(), crypto::Seed::Size);
	kc.db->Put(leveldb::WriteOptions(), DB_KEY_SEED, encrypted_seed.serialize_to_string());

	return kc;
}

Keychain Keychain::open(std::filesystem::path path, crypto::PasswordHash&& pw_hash) {
	Keychain kc;
	kc.tec = std::move(crypto::TimedEncryptionKey(std::move(pw_hash)));

	auto db_path = path / "db";
	leveldb::Status status = leveldb::DB::Open(leveldb::Options(), db_path.string(), &kc.db);
	if (!status.ok()) {
		throw std::runtime_error("could not open db");
	}

	return kc;
}
