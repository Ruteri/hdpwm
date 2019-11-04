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

#include <string>
#include <memory>

namespace leveldb {
class DB;
class Status;
class Slice;
struct Options;
struct ReadOptions;
struct WriteOptions;
} // namespace leveldb

namespace keychain {

class DB {
	leveldb::DB *db;

public:
	virtual ~DB();

	virtual leveldb::Status Get(const leveldb::ReadOptions& options, const leveldb::Slice& key, std::string* value);
	virtual leveldb::Status Put(const leveldb::WriteOptions& options, const leveldb::Slice& key, const leveldb::Slice& value);

	static std::unique_ptr<DB> Open(const leveldb::Options& options, const std::string& name);
};

} // namespace keychain
