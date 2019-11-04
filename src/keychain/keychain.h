#pragma once

#include <src/keychain/keychain_entry.h>

#include <src/crypto/structs.h>
#include <src/crypto/timed_encryption_key.h>

#include <filesystem>

namespace leveldb {
class DB;
}

class Keychain {
  protected:
	std::filesystem::path data_path;

	leveldb::DB *db;
	crypto::TimedEncryptionKey tec;

  public:
	Keychain() = default;
	~Keychain();

	Keychain(const Keychain &) = delete;
	Keychain &operator=(const Keychain &) = delete;

	Keychain(Keychain &&other);
	Keychain &operator=(Keychain &&other);

	static std::unique_ptr<Keychain> initialize_with_seed(
	    std::filesystem::path path, crypto::Seed &&seed, crypto::PasswordHash &&pw_hash);
	static std::unique_ptr<Keychain> open(std::filesystem::path path, crypto::PasswordHash pw_hash);

	std::string get_data_dir_path() const { return data_path.string(); }
	KeychainDirectory::ptr get_root_dir() const;

	crypto::DerivationPath get_next_derivation_path();

	void save_entries(KeychainDirectory::ptr root);

	static utils::sensitive_string encode_secret(
	    unsigned char *in_data, size_t in_size, size_t out_size);
	utils::sensitive_string derive_secret(const crypto::DerivationPath &dpath);
};
