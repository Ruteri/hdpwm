#pragma once

#include <src/crypto/structs.h>
#include <src/crypto/timed_encryption_key.h>

#include <leveldb/db.h>

#include <filesystem>

struct KeychainEntry {
	std::string title;
	std::string details;

	// TODO: maybe should hold encrypted secret? Or can be queried from DB each time
};

class Keychain {
	std::filesystem::path data_path;

	leveldb::DB* db;
	crypto::TimedEncryptionKey tec;

	Keychain() = default;

	Keychain(const Keychain&) = delete;
	Keychain& operator=(const Keychain&) = delete;

public:
	Keychain(Keychain&& other) {
		this->data_path = std::move(other.data_path);
		this->db = other.db;
		other.db = nullptr;
		this->tec = std::move(other.tec);
	}

	Keychain& operator=(Keychain&& other) {
		this->data_path = std::move(other.data_path);
		this->db = other.db;
		other.db = nullptr;
		this->tec = std::move(other.tec);
		return *this;
	}

	~Keychain() { if (this->db) delete this->db; }

	static Keychain initialize_with_seed(std::filesystem::path path, crypto::Seed&& seed, crypto::PasswordHash&& pw_hash);
	static Keychain open(std::filesystem::path path, crypto::PasswordHash&& pw_hash);

	std::string get_data_dir_path() const { return data_path.string(); }
	std::vector<KeychainEntry> get_entries() const {
		return {{"Stub title", "description of the stub title"}};
	}
};
