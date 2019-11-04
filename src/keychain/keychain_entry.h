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

namespace keychain {

struct EntryMeta {
	std::string name;
	std::string details;
	crypto::DerivationPath dpath;
};

struct DirectoryMeta {
	std::string name;
	std::string details;

	// TODO: directories should have their own root keys derived from master key
};

struct Directory;

struct Entry {
	using ptr = std::shared_ptr<Entry>;

	EntryMeta meta;
	std::weak_ptr<Directory> parent_dir;

	Entry(const EntryMeta &meta, std::weak_ptr<Directory> parent_dir) :
	    meta(meta), parent_dir(parent_dir) {}
};

struct Directory {
	using ptr = std::shared_ptr<Directory>;

	DirectoryMeta meta;
	// std::weak_ptr<Directory> parent;

	std::vector<ptr> dirs = {};
	std::vector<Entry::ptr> entries = {};

	int dir_level;
	bool is_open = false;

	Directory(const DirectoryMeta &meta, int dir_level) : meta(meta), dir_level(dir_level) {}

	Directory(const DirectoryMeta &meta, const ptr &parent) :
	    meta(meta), dir_level(parent->dir_level + 1) {}
};

using AnyKeychainPtr = std::variant<Entry::ptr, Directory::ptr>;

Entry::ptr deserialize_entry(const nlohmann::json &data, std::weak_ptr<Directory> parent);
Directory::ptr deserialize_directory(const nlohmann::json &data, int dir_level);

nlohmann::json serialize_entry(Entry::ptr entry);
nlohmann::json serialize_directory(Directory::ptr dir);

std::vector<AnyKeychainPtr> flatten_dirs(Directory::ptr root);

} // namespace keychain
