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

#include <src/keychain/keychain_entry.h>

#include <external/nlohmann/json_single_include.h>
using json = nlohmann::json;

#include <external/catch2/catch.hpp>

using keychain::EntryMeta;
using keychain::Entry;
using keychain::DirectoryMeta;
using keychain::Directory;
using keychain::AnyKeychainPtr;

using keychain::deserialize_entry;
using keychain::serialize_entry;

using keychain::deserialize_directory;
using keychain::serialize_directory;

using keychain::flatten_dirs;

TEST_CASE( "deserializing entries works as intended", "[keychain_entry_deser]" ) {
	json data = json::parse(R"({ "name": "name", "details": "details", "derivation_path": 5 })");

	auto parent_ptr = std::make_shared<Directory>(DirectoryMeta{}, nullptr);
	auto entry = deserialize_entry(data, parent_ptr);

	REQUIRE( entry->meta.name == "name" );
	REQUIRE( entry->meta.details == "details" );
	REQUIRE( entry->meta.dpath.seed == 5 );
	REQUIRE( entry->parent_dir.lock() == parent_ptr );

	REQUIRE( serialize_entry(entry) == data );
}

TEST_CASE( "deserializing drectories works as intended", "[keychain_dir_deser]" ) {
	json data = json::parse(R"({ "name": "dir1", "details": "details1", "dirs": [{"name": "dir2", "details": "details2", "dirs": [], "entries": [{"name": "entry1", "details": "entry_details1", "derivation_path": 6}]}], "entries": [{"name": "entry2", "details": "entry_details2", "derivation_path": 7}] })");

	auto dir = deserialize_directory(data, nullptr);

	REQUIRE( dir->meta.name == "dir1" );
	REQUIRE( dir->meta.details == "details1" );
	REQUIRE( !dir->parent_dir.lock() );
	REQUIRE( dir->dirs.size() == 1 );
	REQUIRE( dir->entries.size() == 1 );

	auto dir2 = dir->dirs[0];
	REQUIRE( dir2->meta.name == "dir2" );
	REQUIRE( dir2->meta.details == "details2" );
	REQUIRE( dir2->parent_dir.lock() == dir );
	REQUIRE( dir2->dirs.size() == 0 );
	REQUIRE( dir2->entries.size() == 1 );

	auto entry1 = dir2->entries[0];
	REQUIRE( entry1->meta.name == "entry1" );
	REQUIRE( entry1->meta.details == "entry_details1" );
	REQUIRE( entry1->meta.dpath.seed == 6 );
	REQUIRE( entry1->parent_dir.lock() == dir2 );

	auto entry2 = dir->entries[0];
	REQUIRE( entry2->meta.name == "entry2" );
	REQUIRE( entry2->meta.details == "entry_details2" );
	REQUIRE( entry2->meta.dpath.seed == 7 );
	REQUIRE( entry2->parent_dir.lock() == dir );

	REQUIRE( serialize_directory(dir) == data );
}

TEST_CASE( "directory deep copying works as intended", "[keychain_dir_deep_copy]" ) {
	json data = json::parse(R"({ "name": "dir1", "details": "details1", "dirs": [{"name": "dir2", "details": "details2", "dirs": [], "entries": [{"name": "entry1", "details": "entry_details1", "derivation_path": 6}]}], "entries": [{"name": "entry2", "details": "entry_details2", "derivation_path": 7}] })");

	auto root = deserialize_directory(data, 0);
	auto dir1 = root->dirs[0];

	// Copy dir1 w/r to dir1
	auto copied_root = deep_copy_directory(root, root->dirs[0]);

	REQUIRE( copied_root->parent_dir.lock() == dir1 );
	REQUIRE( copied_root->dir_level == 2 );
	REQUIRE( copied_root->dirs[0]->meta.name == dir1->meta.name );
	REQUIRE( copied_root->dirs[0] != dir1 ); // Should be a copy
	REQUIRE( copied_root->dirs[0]->parent_dir.lock() == copied_root );
	REQUIRE( copied_root->dirs[0]->dir_level == 3 );
	REQUIRE( copied_root->entries[0]->meta.name == root->entries[0]->meta.name );
	REQUIRE( copied_root->entries[0]->parent_dir.lock() == copied_root );

	REQUIRE( serialize_directory(copied_root) == data );
}

TEST_CASE( "flattening works as intended", "[keychain_flatten]" ) {
	json data = json::parse(R"({ "name": "dir1", "details": "details1", "dirs": [{"name": "dir2", "details": "details2", "dirs": [], "entries": [{"name": "entry1", "details": "entry_details1", "derivation_path": 6}]}], "entries": [{"name": "entry2", "details": "entry_details2", "derivation_path": 7}] })");

	auto root = deserialize_directory(data, 0);

	{
		std::vector<AnyKeychainPtr> flatten_result = flatten_dirs(root);
		REQUIRE( flatten_result.size() == 1 );
		auto flattened_root = std::get<Directory::ptr>(flatten_result[0]);
		REQUIRE( flattened_root->meta.name == "dir1" );
		REQUIRE( !flattened_root->parent_dir.lock() );
	}

	{
		root->is_open = true;
		std::vector<AnyKeychainPtr> flatten_result = flatten_dirs(root);
		REQUIRE( flatten_result.size() == 3 );

		auto flattened_root = std::get<Directory::ptr>(flatten_result[0]);
		REQUIRE( flattened_root->meta.name == "dir1" );
		REQUIRE( !flattened_root->parent_dir.lock() );

		auto flattened_dir2 = std::get<Directory::ptr>(flatten_result[1]);
		REQUIRE( flattened_dir2->meta.name == "dir2" );
		REQUIRE( flattened_dir2->parent_dir.lock() == flattened_root );

		auto flattened_entry2 = std::get<Entry::ptr>(flatten_result[2]);
		REQUIRE( flattened_entry2->meta.name == "entry2" );
		REQUIRE( flattened_entry2->parent_dir.lock() == flattened_root );
	}

	{
		root->is_open = true;
		root->dirs[0]->is_open = true;

		std::vector<AnyKeychainPtr> flatten_result = flatten_dirs(root);
		REQUIRE( flatten_result.size() == 4 );

		auto flattened_root = std::get<Directory::ptr>(flatten_result[0]);
		REQUIRE( flattened_root->meta.name == "dir1" );
		REQUIRE( !flattened_root->parent_dir.lock() );

		auto flattened_dir2 = std::get<Directory::ptr>(flatten_result[1]);
		REQUIRE( flattened_dir2->meta.name == "dir2" );
		REQUIRE( flattened_dir2->parent_dir.lock() == flattened_root );

		auto flattened_entry1 = std::get<Entry::ptr>(flatten_result[2]);
		REQUIRE( flattened_entry1->meta.name == "entry1" );
		REQUIRE( flattened_entry1->parent_dir.lock() == flattened_dir2 );

		auto flattened_entry2 = std::get<Entry::ptr>(flatten_result[3]);
		REQUIRE( flattened_entry2->meta.name == "entry2" );
		REQUIRE( flattened_entry2->parent_dir.lock() == flattened_root );
	}
}
