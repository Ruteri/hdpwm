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
	using ptr = std::shared_ptr<KeychainEntry>;

	KeychainEntryMeta meta;
	std::weak_ptr<KeychainDirectory> parent_dir;

	KeychainEntry(const KeychainEntryMeta &meta, std::weak_ptr<KeychainDirectory> parent_dir) :
	    meta(meta), parent_dir(parent_dir) {}
};

struct KeychainDirectory {
	using ptr = std::shared_ptr<KeychainDirectory>;

	KeychainDirectoryMeta meta;
	// std::weak_ptr<KeychainDirectory> parent;

	std::vector<ptr> dirs = {};
	std::vector<KeychainEntry::ptr> entries = {};

	int dir_level;
	bool is_open = false;

	KeychainDirectory(const KeychainDirectoryMeta &meta, int dir_level) :
	    meta(meta), dir_level(dir_level) {}

	KeychainDirectory(const KeychainDirectoryMeta &meta, const ptr &parent) :
	    meta(meta), dir_level(parent->dir_level + 1) {}
};

using AnyKeychainPtr = std::variant<KeychainEntry::ptr, KeychainDirectory::ptr>;

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
	KeychainDirectory::ptr get_root_dir() const;
	void save_entries(KeychainDirectory::ptr root);

	static std::vector<AnyKeychainPtr> flatten_dirs(KeychainDirectory::ptr root);
};
