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

#include <src/crypto/mnemonic.h>

#include <src/crypto/mnemonic-wordlist.cpp>
#include <src/crypto/utils.h>
#include <src/utils/utils.h>

#include <external/cryptopp/osrng.h>
#include <external/cryptopp/pwdbased.h>
#include <external/cryptopp/sha.h>

#include <stdexcept>

namespace crypto {

constexpr int PBKDF2_ITERATION_COUNT = 2048;

uint8_t extract_bit(const CryptoPP::byte *buffer, int cb) {
	const CryptoPP::byte byte = buffer[cb / 8];
	const uint8_t mask = 1 << (7 - (cb % 8));
	return byte & mask;
}

utils::sensitive_string word_at(int index) {
	return mnemonic_dictionary[index % mnemonic_dictionary.size()];
}

int find_word_index(const utils::sensitive_string &word) {
	auto it = std::lower_bound(mnemonic_dictionary.begin(), mnemonic_dictionary.end(), word);
	if (it == mnemonic_dictionary.end() || *it != word) {
		throw std::runtime_error("unknown word");
	}

	return it - mnemonic_dictionary.begin();
}

std::vector<int> bitsplit_11(const CryptoPP::byte *buffer, int size) {
	std::vector<int> rv;
	rv.reserve(size / 11);
	for (int sb = 0; sb <= size - 11; sb += 11) {
		unsigned int v = 0;

		for (int cb = sb; cb < sb + 11; ++cb) {
			uint8_t extracted_value = extract_bit(buffer, cb);
			v = (v << 1) | !!extracted_value;
		}
		rv.push_back(v);
	}

	return rv;
}

std::vector<utils::sensitive_string> get_words_from_indices(const std::vector<int> &indices) {
	std::vector<utils::sensitive_string> rv;
	rv.reserve(indices.size());
	for (auto index : indices) {
		rv.push_back(word_at(index));
	}

	return rv;
}

utils::sensitive_string get_random_word() {
	CryptoPP::byte seed[2];
	CryptoPP::NonblockingRng rng;
	rng.GenerateBlock(seed, sizeof(seed));
	uint16_t random_index = seed[0] << 8 | seed[1];
	return word_at(random_index);
}

std::vector<utils::sensitive_string> generate_mnemonic(int entropy_size) {

	constexpr int CHECKSUM_MAX_SIZE = 1;

	CryptoPP::byte *seed = new CryptoPP::byte[entropy_size + CHECKSUM_MAX_SIZE];

	CryptoPP::NonblockingRng rng;
	rng.GenerateBlock(seed, entropy_size);

	CryptoPP::SHA256 checksum_digester;
	checksum_digester.CalculateTruncatedDigest(seed + entropy_size, 1, seed, entropy_size);

	auto indices = bitsplit_11(seed, entropy_size * 8 + entropy_size / 4);
	auto words = get_words_from_indices(indices);

	utils::sensitive_string extra_word = get_random_word();
	words.push_back(extra_word);

	delete[] seed;
	return words;
}

Seed mnemonic_to_seed(const std::vector<utils::sensitive_string> &words) {
	int words_len = 0;
	for (auto word : words) {
		words_len += word.size();
	}

	CryptoPP::byte *mnemonic = new CryptoPP::byte[words_len];
	CryptoPP::byte *pw_ptr = mnemonic;
	for (auto word : words) {
		std::strncpy(reinterpret_cast<char *>(pw_ptr), word.data, word.size());
		pw_ptr += word.size();
	}

	const CryptoPP::byte salt[] = "ob1Ofabex?reg+ojAfKosh89OkEgUsvojbeurOv7knok";

	Seed seed;
	CryptoPP::PKCS5_PBKDF2_HMAC<CryptoPP::SHA512> pbkdf;
	CryptoPP::byte unused = 0;

	pbkdf.DeriveKey(reinterpret_cast<CryptoPP::byte *>(seed.data()), CryptoPP::SHA512::DIGESTSIZE,
	    unused, mnemonic, words_len, salt, sizeof(salt), PBKDF2_ITERATION_COUNT);

	delete[] mnemonic;
	return seed;
}

std::vector<utils::sensitive_string> split_mnemonic_words(const utils::sensitive_string &mnemonic) {
	std::vector<utils::sensitive_string> ret;
	ret.reserve(25); // enough for longest allowed mnemonic
	utils::sensitive_string c_word(12);
	for (size_t i = 0; i < mnemonic.size(); ++i) {
		if (mnemonic[i] == ' ') {
			ret.push_back(std::move(c_word));
			c_word = utils::sensitive_string(12);
		} else {
			c_word.push_back(mnemonic[i]);
		}
	}

	ret.push_back(std::move(c_word));
	return ret;
}

} // namespace crypto
