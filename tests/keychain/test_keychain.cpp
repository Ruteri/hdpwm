#include <src/keychain/keychain.h>
#include <src/keychain/db.h>

#include <src/keychain/db.h>

#include <leveldb/db.h>

#include <external/nlohmann/json_single_include.h>
using json = nlohmann::json;

#include <external/catch2/catch.hpp>

class KeychainMock: public keychain::Keychain {
public:
	void set_db(std::unique_ptr<keychain::DB> db) { this->db = std::move(db); }
	void set_ec(crypto::PasswordHash pw_hash) { this->tec = crypto::TimedEncryptionKey(pw_hash); }

};

class DBMock: public keychain::DB {
public:
	~DBMock() final {}

	int Get_call_count = 0;
	std::vector<std::tuple<leveldb::ReadOptions, leveldb::Slice, std::string*>> Get_calls;
	std::function<leveldb::Status(const leveldb::ReadOptions&, const leveldb::Slice&, std::string*)> Get_mock_fn;
	leveldb::Status Get(const leveldb::ReadOptions& options, const leveldb::Slice& key, std::string* value) final {
		++Get_call_count;
		Get_calls.emplace_back( std::make_tuple(options, key, value) );
		return Get_mock_fn(options, key, value);
	}

	int Put_call_count = 0;
	std::vector<std::tuple<leveldb::WriteOptions, std::pair<leveldb::Slice, leveldb::Slice>>> Put_calls;
	std::function<leveldb::Status(const leveldb::WriteOptions&, const leveldb::Slice&, const leveldb::Slice&)> Put_mock_fn =
	    [](const leveldb::WriteOptions&, const leveldb::Slice&, const leveldb::Slice&) { return leveldb::Status(); };
	leveldb::Status Put(const leveldb::WriteOptions& options, const leveldb::Slice& key, const leveldb::Slice& value) final {
		++Put_call_count;
		Put_calls.push_back({ options, {key, value} });
		return Put_mock_fn(options, key, value);
	}
};

json sample_entries = json::parse(R"({ "name": "dir1", "details": "details1", "dirs": [{"name": "dir2", "details": "details2", "dirs": [], "entries": [{"name": "entry1", "details": "entry_details1", "derivation_path": 6}]}], "entries": [{"name": "entry2", "details": "entry_details2", "derivation_path": 7}] })");

struct seed_data_s {
	std::string seed;
	std::string expected;
	int size;
};

TEST_CASE( "secret encoding works as intended", "[keychain_secret_encoding]" ) {

	std::vector<seed_data_s> data = {
		{ "cb167a75be85dda7988b628dd9e5ffbe13d3acd86fcf4cc0e742de17d80b3e25fb9781acc11821301f15bb2728225093c9b302dca2e05239785952218c3da735", R"(MkAsM%uZXu)", 10 },
		{ "cb167a75be85dda7988b628dd9e5ffbe13d3acd86fcf4cc0e742de17d80b3e25fb9781acc11821301f15bb2728225093c9b302dca2e05239785952218c3da735", R"(MkAsM%uZXu*HKQTQj.g8~)", 21 },
		{ "cb167a75be85dda7988b628dd9e5ffbe13d3acd86fcf4cc0e742de17d80b3e25fb9781acc11821301f15bb2728225093c9b302dca2e05239785952218c3da735", R"(MkAsM%uZXu*HKQTQj.g8~b)", 22 },
		{ "cb167a75be85dda7988b628dd9e5ffbe13d3acd86fcf4cc0e742de17d80b3e25fb9781acc11821301f15bb2728225093c9b302dca2e05239785952218c3da735", R"(MkAsM%uZXu*HKQTQj.g8~b)", 23 },
	};

	for (auto &tc : data) {
		auto seed = crypto::deserialize<crypto::Seed>(tc.seed);
		auto encoded = keychain::Keychain::encode_secret(seed.data(), seed.size(), tc.size);
		REQUIRE( static_cast<std::string>(encoded) == tc.expected );
	}
}

TEST_CASE( "secrets are derived properly", "[keychain_derive_secret]" ) {
	auto db = new DBMock();

	db->Get_mock_fn = [](const leveldb::ReadOptions&, const leveldb::Slice& key, std::string* value) {
		REQUIRE( key.ToString() == "seed" );
		*value = "a0727f73ff7cb6eea580b5e808b26e28110add5a34481a7e2ac282e649c7d6feccf870a9448b901087adc0a224059e2855fcfe221c5db00dc598aad29c2593f6";
		return leveldb::Status();
	};

	KeychainMock kc;
	kc.set_db(std::unique_ptr<keychain::DB>(db));

	auto password_hash = crypto::deserialize<crypto::PasswordHash>("99e2177f9e650b9a38c6b72f9196fc46f87e80b9655002c70e6849bdfd14210f");
	kc.set_ec(password_hash);

	auto rv = kc.derive_secret({ 1 });

	REQUIRE( static_cast<std::string>(rv) == "XhGEe*rhgV" );
}

TEST_CASE( "entries are saved as expected", "[keychain_save_entries]" ) {
	auto db = new DBMock();

	KeychainMock kc;
	kc.set_db(std::unique_ptr<keychain::DB>(db));

	auto root = keychain::deserialize_directory(sample_entries, 0);

	kc.save_entries(root);

	REQUIRE( db->Get_call_count == 0 );
	REQUIRE( db->Put_call_count == 1 );
	REQUIRE( std::get<1>(db->Put_calls[0]) == std::pair<leveldb::Slice, leveldb::Slice>{"entries", sample_entries.dump()} );
}

TEST_CASE( "entries' root is created properly", "[keychain_get_root]" ) {
	auto db = new DBMock();

	db->Get_mock_fn = [](const leveldb::ReadOptions&, const leveldb::Slice& key, std::string* value) {
		REQUIRE( key.ToString() == "entries" );
		*value = sample_entries.dump();
		return leveldb::Status();
	};

	KeychainMock kc;
	kc.set_db(std::unique_ptr<keychain::DB>(db));

	auto root = kc.get_root_dir();

	/* do basic checks, deserialization is covered by test_keychain_entry.cpp */
	REQUIRE( root->meta.name == "dir1" );
	REQUIRE( root->dirs.size() == 1 );
	REQUIRE( root->entries.size() == 1 );

	REQUIRE( root->dirs[0]->meta.name == "dir2" );
	REQUIRE( root->dirs[0]->dirs.size() == 0 );
	REQUIRE( root->dirs[0]->entries.size() == 1 );
	REQUIRE( root->dirs[0]->entries[0]->meta.name == "entry1" );

	REQUIRE( root->entries[0]->meta.name == "entry2" );
}
