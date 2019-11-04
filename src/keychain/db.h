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
