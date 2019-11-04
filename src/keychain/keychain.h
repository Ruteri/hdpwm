#pragma once

#include <src/crypto/structs.h>
#include <src/crypto/timed_encryption_key.h>

#include <filesystem>
#include <variant>

namespace leveldb {
class DB;
}

// helper for visitors
template <class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template <class... Ts> overloaded(Ts...)->overloaded<Ts...>;

struct DerivationPath {
	int seed;
};

struct KeychainEntryMeta {
	std::string name;
	std::string details;
	DerivationPath dpath;
};

struct KeychainDirectoryMeta {
	std::string name;
	std::string details;

	// TODO: directories should have their own root keys derived from master key
};

struct KeychainDirectory;

struct KeychainEntry {
	KeychainEntryMeta meta;
	KeychainDirectory *parent_dir;

	KeychainEntry(const KeychainEntryMeta &meta, KeychainDirectory *parent_dir) :
	    meta(meta), parent_dir(parent_dir) {}
};

struct KeychainDirectory {
	KeychainDirectoryMeta meta;
	KeychainDirectory *parent;

	std::vector<KeychainDirectory> dirs = {};
	std::vector<KeychainEntry> entries = {};

	int dir_level;
	bool is_open = false;

	KeychainDirectory(const KeychainDirectoryMeta &meta, KeychainDirectory *parent) :
	    meta(meta), parent(parent) {
		dir_level = parent ? parent->dir_level + 1 : 0;
	}
};

std::vector<std::variant<KeychainDirectory *, KeychainEntry *>> flatten_dirs(
    std::shared_ptr<KeychainDirectory> root);

class Keychain {
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
	std::shared_ptr<KeychainDirectory> get_root_dir() const {
		// TODO: should be generated from data in DB (for now mock)
		std::shared_ptr<KeychainDirectory> root =
		    std::make_unique<KeychainDirectory>(KeychainDirectoryMeta{"/", ""}, nullptr);
		KeychainDirectory *root_raw_ptr = static_cast<KeychainDirectory *>(root.get());

		root->dirs.emplace_back(KeychainDirectoryMeta{"General", ""}, root_raw_ptr);
		root->dirs[0].dir_level = 1;
		root->dirs[0].entries.emplace_back(
		    KeychainEntryMeta{"some password", "password to some site", {0}}, &(root->dirs[0]));
		root->entries.emplace_back(
		    KeychainEntryMeta{"other password", "password to some other site", {1}}, root_raw_ptr);
		root->is_open = true;
		return root;
	}
};
