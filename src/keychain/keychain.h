#pragma once

#include <src/crypto/structs.h>
#include <src/crypto/timed_encryption_key.h>

#include <leveldb/db.h>

#include <filesystem>

class Keychain {
	leveldb::DB* db;
	crypto::TimedEncryptionKey tec;

	Keychain() = default;

	Keychain(const Keychain&) = delete;
	Keychain& operator=(const Keychain&) = delete;

public:
	Keychain(Keychain&& other) {
		this->db = other.db;
		other.db = nullptr;
		this->tec = std::move(other.tec);
	}

	Keychain& operator=(Keychain&& other) {
		this->db = other.db;
		other.db = nullptr;
		this->tec = std::move(other.tec);
		return *this;
	}

	~Keychain() { if (this->db) delete this->db; }

	static Keychain initialize_with_seed(std::filesystem::path path, crypto::Seed&& seed, crypto::PasswordHash&& pw_hash);
	static Keychain open(std::filesystem::path path, crypto::PasswordHash&& pw_hash);

};
