#include <src/crypto/crypto.h>

#include <src/utils/utils.h>

#include <external/catch2/catch.hpp>

using crypto::serialize;
using crypto::deserialize;

using crypto::Seed;
using crypto::EncryptedSeed;
using crypto::PasswordHash;

TEST_CASE( "hashes are serialized and deserialized properly", "[hash_serialization]" ) {
	auto expected_password_hash = deserialize<PasswordHash>("99e2177f9e650b9a38c6b72f9196fc46f87e80b9655002c70e6849bdfd14210f");

	REQUIRE( serialize<PasswordHash>(expected_password_hash) == "99e2177f9e650b9a38c6b72f9196fc46f87e80b9655002c70e6849bdfd14210f" );
}

TEST_CASE( "seeds are encrypted properly", "[encrypt_seed]" ) {
	utils::sensitive_string password("password");

	auto expected_password_hash = deserialize<PasswordHash>("5e884898da28047151d0e56f8dc6292773603d0d6aabbdd62a11ef721d1542d8");

	auto seed = deserialize<Seed>("f29b278b6525f2bf2e78c24ed086d58836438509bea3d837e1434ee6d3b082c23e60a199c9eb05dc1f6307bb99aca5025e2241fec580312b0064b375020cc2fd");

	auto expected_encrypted_seed = deserialize<EncryptedSeed>("a0727f73ff7cb6eea580b5e808b26e28110add5a34481a7e2ac282e649c7d6feccf870a9448b901087adc0a224059e2855fcfe221c5db00dc598aad29c2593f6");

	PasswordHash password_hash = crypto::hash_password(password);

	REQUIRE( password_hash._data == expected_password_hash._data );

	EncryptedSeed encrypted_seed = crypto::encrypt_seed(seed, password_hash);

	REQUIRE( encrypted_seed._data == expected_encrypted_seed._data );
}

TEST_CASE( "seeds are decrypted properly", "[decrypt_seed]" ) {
	utils::sensitive_string password("password");

	auto encrypted_seed = deserialize<EncryptedSeed>("430ce4aea8c215883057e47fc5666d6fcd60359334fe7cec828cdedfdaa230e526001cef8a8138b3f317bc573ffbc7b0fd0cb342d9de01b4b7a3fe6ae50e086f");

	auto expected_seed = deserialize<Seed>("11e5bc56329b51d9bbaf93d91d52d6cfc4431b30d068f2a80bf2d9019aafe0d7324d12f8d816b3c1962a3bb00b1fe89b95021fb542b5f5ccf21d0c42e56df1a5");

	PasswordHash password_hash = crypto::hash_password(password);

	Seed seed = crypto::decrypt_seed(encrypted_seed, password_hash);

	REQUIRE( seed._data == expected_seed._data );
}

TEST_CASE( "children are derived properly", "[derive_child]" ) {
	auto password_hash = deserialize<PasswordHash>("99e2177f9e650b9a38c6b72f9196fc46f87e80b9655002c70e6849bdfd14210f");

	auto encrypted_parent_key = deserialize<EncryptedSeed>("430ce4aea8c215883057e47fc5666d6fcd60359334fe7cec828cdedfdaa230e526001cef8a8138b3f317bc573ffbc7b0fd0cb342d9de01b4b7a3fe6ae50e086f");

	auto expected_seed = deserialize<Seed>("cb167a75be85dda7988b628dd9e5ffbe13d3acd86fcf4cc0e742de17d80b3e25fb9781acc11821301f15bb2728225093c9b302dca2e05239785952218c3da735");

	Seed seed = crypto::derive_child(password_hash, encrypted_parent_key, {1});

	REQUIRE( seed._data == expected_seed._data );
}

