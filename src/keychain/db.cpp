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
