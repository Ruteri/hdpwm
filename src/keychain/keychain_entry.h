#pragma once

#include <src/crypto/crypto.h>

#include <external/nlohmann/json_fwd.hpp>

#include <list>
#include <memory>
#include <variant>
#include <vector>

// helper for visitors
template <class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template <class... Ts> overloaded(Ts...)->overloaded<Ts...>;

struct KeychainEntryMeta {
	std::string name;
	std::string details;
	crypto::DerivationPath dpath;
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

KeychainEntry::ptr deserialize_entry(
    const nlohmann::json &data, std::weak_ptr<KeychainDirectory> parent);
KeychainDirectory::ptr deserialize_directory(const nlohmann::json &data, int dir_level);

nlohmann::json serialize_entry(KeychainEntry::ptr entry);
nlohmann::json serialize_directory(KeychainDirectory::ptr dir);

std::vector<AnyKeychainPtr> flatten_dirs(KeychainDirectory::ptr root);
