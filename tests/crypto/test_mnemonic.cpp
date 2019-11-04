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
		REQUIRE( seed._data == tc.expected_seed._data );
	}
}

// TODO: Should throw on invalid seed
TEST_CASE( "seed calculation throws on invalid word", "[seeds_vector_throws_on_invalid]" ) {
	for (auto &im : invalid_mnemonic_test_vector) {
		(void) crypto::mnemonic_to_seed(std::move(im.mnemonic));
	}
}
