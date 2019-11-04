#include <tests/crypto/test_data.cpp>

#include <src/crypto/mnemonic.h>
#include <src/utils/utils.h>

#include <external/catch2/catch.hpp>

void verify_words_are_in_dictionary(const std::vector<std::string>& mnemonic) {
	for (auto word : mnemonic) {
		REQUIRE( crypto::find_word_index(word) >= 0 );
	}
}

TEST_CASE( "12-word mnemonic is generated", "[generate_mnemonic_12w]" ) {
	std::vector<std::string> mn_12w = crypto::generate_mnemonic(16);
	REQUIRE( mn_12w.size() == 12 + 1);
	verify_words_are_in_dictionary(mn_12w);
}

TEST_CASE( "15-word mnemonic is generated", "[generate_mnemonic_15w]" ) {
	std::vector<std::string> mn_15w = crypto::generate_mnemonic(20);
	REQUIRE( mn_15w.size() == 15 + 1 );
	verify_words_are_in_dictionary(mn_15w);
}

TEST_CASE( "18-word mnemonic is generated", "[generate_mnemonic_18w]" ) {
	std::vector<std::string> mn_18w = crypto::generate_mnemonic(24);
	REQUIRE( mn_18w.size() == 18 + 1 );
	verify_words_are_in_dictionary(mn_18w);
}

TEST_CASE( "21-word mnemonic is generated", "[generate_mnemonic_21w]" ) {
	std::vector<std::string> mn_21w = crypto::generate_mnemonic(28);
	REQUIRE( mn_21w.size() == 21 + 1 );
	verify_words_are_in_dictionary(mn_21w);
}

TEST_CASE( "24-word mnemonic is generated", "[generate_mnemonic_24w]" ) {
	std::vector<std::string> mn_24w = crypto::generate_mnemonic(32);
	REQUIRE( mn_24w.size() == 24 + 1 );
	verify_words_are_in_dictionary(mn_24w);
}

TEST_CASE( "seeds are calculated properly", "[seeds_vector]" ) {
	for (auto &tc : mnemonic_test_vector) {
		auto seed = crypto::mnemonic_to_seed(std::move(tc.mnemonic));
		REQUIRE( seed._data == tc.expected_seed );
	}
}

// TODO: Should throw on invalid seed
TEST_CASE( "seed calculation throws on invalid word", "[seeds_vector_throws_on_invalid]" ) {
	for (auto &im : invalid_mnemonic_test_vector) {
		(void) crypto::mnemonic_to_seed(std::move(im.mnemonic));
	}
}

TEST_CASE( "seeds are encrypted properly", "[encrypt_seed]" ) {
	utils::sensitive_string password("password");

	crypto::PasswordHash expected_password_hash;
	auto expected_password_hash_bytes = utils::unhexify("99e2177f9e650b9a38c6b72f9196fc46f87e80b9655002c70e6849bdfd14210f");
	std::copy(expected_password_hash_bytes.begin(), expected_password_hash_bytes.end(), expected_password_hash.begin());

	crypto::Seed seed;
	auto seed_bytesv = utils::unhexify("f29b278b6525f2bf2e78c24ed086d58836438509bea3d837e1434ee6d3b082c23e60a199c9eb05dc1f6307bb99aca5025e2241fec580312b0064b375020cc2fd");
	std::copy(seed_bytesv.begin(), seed_bytesv.end(), seed.begin());

	crypto::EncryptedSeed expected_encrypted_seed;
	auto expected_seed_bytesv = utils::unhexify("430ce4aea8c215883057e47fc5666d6fcd60359334fe7cec828cdedfdaa230e526001cef8a8138b3f317bc573ffbc7b0fd0cb342d9de01b4b7a3fe6ae50e086f");
	std::copy(expected_seed_bytesv.begin(), expected_seed_bytesv.end(), expected_encrypted_seed.begin());


	crypto::PasswordHash password_hash = std::move(crypto::hash_password(std::move(password)));

	REQUIRE( password_hash._data == expected_password_hash._data );

	crypto::EncryptedSeed encrypted_seed = crypto::encrypt_seed(std::move(seed), std::move(password_hash));

	REQUIRE( encrypted_seed._data == expected_encrypted_seed._data );
}

TEST_CASE( "seeds are decrypted properly", "[decrypt_seed]" ) {
	utils::sensitive_string password("password");

	crypto::EncryptedSeed encrypted_seed;
	auto seed_bytesv = utils::unhexify("430ce4aea8c215883057e47fc5666d6fcd60359334fe7cec828cdedfdaa230e526001cef8a8138b3f317bc573ffbc7b0fd0cb342d9de01b4b7a3fe6ae50e086f");
	std::copy(seed_bytesv.begin(), seed_bytesv.end(), encrypted_seed.begin());

	crypto::Seed expected_seed;
	auto expected_seed_bytesv = utils::unhexify("f29b278b6525f2bf2e78c24ed086d58836438509bea3d837e1434ee6d3b082c23e60a199c9eb05dc1f6307bb99aca5025e2241fec580312b0064b375020cc2fd");
	std::copy(expected_seed_bytesv.begin(), expected_seed_bytesv.end(), expected_seed.begin());


	crypto::PasswordHash password_hash = std::move(crypto::hash_password(std::move(password)));

	crypto::Seed seed = crypto::decrypt_seed(std::move(encrypted_seed), std::move(password_hash));

	REQUIRE( seed._data == expected_seed._data );
}
