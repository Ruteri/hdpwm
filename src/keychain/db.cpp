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

#include <src/keychain/db.h>

#include <leveldb/db.h>

namespace keychain {

DB::~DB() {
	if (this->db) delete this->db;
	this->db = nullptr;
}

leveldb::Status DB::Get(const leveldb::ReadOptions& options, const leveldb::Slice& key, std::string* value) {
	return this->db->Get(options, key, value);
}

leveldb::Status DB::Put(const leveldb::WriteOptions& options, const leveldb::Slice& key, const leveldb::Slice& value) {
	return this->db->Put(options, key, value);
}

std::unique_ptr<DB> DB::Open(const leveldb::Options& options, const std::string& name) {
	auto db = std::make_unique<DB>();
	auto status = leveldb::DB::Open(options, name, &db->db);
	if (!status.ok()) {
		throw std::runtime_error("could not open db");
	}

	return db;
}

} // namespace keychain
