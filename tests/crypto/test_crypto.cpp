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

#include <src/crypto/crypto.h>

#include <src/utils/utils.h>

#include <external/catch2/catch.hpp>

using crypto::serialize;
using crypto::deserialize;

using crypto::B64EncodedText;
using crypto::base64_encode;
using crypto::base64_decode;
using crypto::as_string;
using crypto::as_encoded;

using crypto::encrypt;
using crypto::decrypt;

using crypto::Seed;
using crypto::Ciphertext;
using crypto::EncryptedSeed;
using crypto::EncryptionKey;
using crypto::PasswordHash;

TEST_CASE( "hashes are serialized and deserialized properly", "[hash_serialization]" ) {
	auto expected_password_hash = deserialize<PasswordHash>("99e2177f9e650b9a38c6b72f9196fc46f87e80b9655002c70e6849bdfd14210f");

	REQUIRE( serialize<PasswordHash>(expected_password_hash) == "99e2177f9e650b9a38c6b72f9196fc46f87e80b9655002c70e6849bdfd14210f" );
}

TEST_CASE( "recoding as string and as vector works as intended", "[crypto_b64_recoding]" ) {
	struct tc_case {
		std::string decoded;
		B64EncodedText encoded;
	};

	std::vector<tc_case> cases = {
		{ "", {} },
		{ "sometext", {"sometext"} },
		{ "someothertext", {"someothertext"} },
	};

	for (auto &tc : cases) {
		REQUIRE( as_encoded(tc.decoded) == tc.encoded );
		REQUIRE( as_string(tc.encoded) == tc.decoded );
	}
}

TEST_CASE( "base64 encoding and decoding works as intended", "[base64_coding]" ) {
	struct tc_case {
		std::string decoded;
		B64EncodedText encoded;
	};

	std::vector<tc_case> cases = {
		{ "", {} },
		{ "sometext", as_encoded("c29tZXRleHQ=") },
		{ "someothertext", as_encoded("c29tZW90aGVydGV4dA==") },
		{ "someothertextp", as_encoded("c29tZW90aGVydGV4dHA=") },
		{ "someothertextpp", as_encoded("c29tZW90aGVydGV4dHBw") },
		{ "someothertextppp", as_encoded("c29tZW90aGVydGV4dHBwcA==") },
		{ "someothertextpppp", as_encoded("c29tZW90aGVydGV4dHBwcHA=") },
	};

	for (auto &tc : cases) {
		INFO( "Case " << tc.decoded );
		REQUIRE( base64_encode(tc.decoded) == tc.encoded );
		REQUIRE( base64_decode(tc.encoded) == tc.decoded );
	}
}

utils::sensitive_string unhexify(std::string hexstr) {
}

TEST_CASE( "string encryption and decryption works as intended", "[crypto_encrypt_decrypt_string]" ) {
	struct ec_case {
		std::string key;
		B64EncodedText plaintext;
		Ciphertext ciphertext;
	};

	std::vector<ec_case> cases = {
		// {"99e2177f9e650b9a38c6b72f9196fc46f87e80b9655002c70e6849bdfd14210f", {""}, {""}},
		// {"99e2177f9e650b9a38c6b72f9196fc46f87e80b9655002c70e6849bdfd14210f", {"\0\0", 2}, deserialize<Ciphertext>("b197")},
		// {"99e2177f9e650b9a38c6b72f9196fc46f87e80b9655002c70e6849bdfd14210f", crypto::base64_encode(std::string(R"({"some_key": "some json string"})")), deserialize<Ciphertext>("d4ee895fafd5d65b461d525d70b3f1d12b34964e3241cc9a5edeed792bf95c82562522ffdaf7f26ab99160b9")},
	};

	int i = 0;
	for (auto &tc : cases) {
		INFO( "Case " << i++ );
		auto key = deserialize<EncryptionKey>(tc.key);
		REQUIRE( encrypt(key, tc.plaintext) == tc.ciphertext );
		REQUIRE( decrypt(key, tc.ciphertext) == tc.plaintext );
	}
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

