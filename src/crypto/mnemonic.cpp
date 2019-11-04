#include <src/crypto/mnemonic-wordlist.cpp>

#include <external/cryptopp/osrng.h>
#include <external/cryptopp/pwdbased.h>
#include <external/cryptopp/sha.h>

#include <iomanip>
#include <iostream>
#include <stdexcept>

namespace {

// WIP debug output until compliant with BIP39
void print_bytes(const CryptoPP::byte *buffer, int size) {
	std::cout << "0x";
	for (int i = 0; i < size; ++i) {
		std::cout << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(buffer[i]);
	}

	std::cout << std::endl;
}

} // annonymous namespace (utils)

namespace crypto {

constexpr int PBKDF2_ITERATION_COUNT = 2048;

uint8_t extract_bit(const CryptoPP::byte *buffer, int cb) {
	const CryptoPP::byte byte = buffer[cb/8];
	const uint8_t mask = 1 << (7 - (cb % 8));
	return byte & mask;
}

std::string word_at(int index) {
	return mnemonic_dictionary[index];
}

int find_word_index(std::string word) {
	auto it = std::lower_bound(mnemonic_dictionary.begin(), mnemonic_dictionary.end(), word);
	if (it == mnemonic_dictionary.end() || *it != word) {
		throw new std::runtime_error("unknown word");
	}

	return it - mnemonic_dictionary.begin();
}

std::vector<int> bitsplit_11(const CryptoPP::byte *buffer, int size) {
	std::vector<int> rv;
	rv.reserve(size/11);
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

std::vector<std::string> get_words_from_indices(std::vector<int> indices) {
	std::vector<std::string> rv;
	rv.reserve(indices.size());
	for (auto index : indices) {
		rv.push_back(word_at(index));
	}

	return rv;
}

std::vector<std::string> generate_mnemonic(int entropy_size) {

	constexpr int CHECKSUM_MAX_SIZE = 1;

	CryptoPP::byte *seed = new CryptoPP::byte[entropy_size + CHECKSUM_MAX_SIZE];

	CryptoPP::NonblockingRng rng;
	rng.GenerateBlock(seed, entropy_size);

	CryptoPP::SHA256 checksum_digester;
	checksum_digester.CalculateTruncatedDigest(seed + entropy_size, 1, seed, entropy_size);

#	ifdef DEBUG
	print_bytes(seed, entropy_size + CHECKSUM_MAX_SIZE);
#	endif

	auto indices = bitsplit_11(seed, entropy_size * 8 + entropy_size / 4);
	auto words = get_words_from_indices(indices);

	delete[] seed;
	return words;
}

// TODO: add user-provided password
std::vector<uint8_t> mnemonic_to_seed(std::vector<std::string> words) {
	int words_len = 0;
	for (auto word : words) {
		words_len += word.size();
	}

	CryptoPP::byte *mnemonic = new CryptoPP::byte[words_len];
	CryptoPP::byte *pw_ptr = mnemonic;
	for (auto word : words) {
		std::strcpy((char *)pw_ptr, word.c_str());
		pw_ptr += word.size();
	}

	CryptoPP::byte salt[] = "mnemonic"; // + user-provided password

    CryptoPP::byte derived[CryptoPP::SHA512::DIGESTSIZE];
	CryptoPP::PKCS5_PBKDF2_HMAC<CryptoPP::SHA512> pbkdf;
	CryptoPP::byte unused = 0;

    pbkdf.DeriveKey(derived, CryptoPP::SHA512::DIGESTSIZE, unused, mnemonic, words_len, salt, sizeof(salt), PBKDF2_ITERATION_COUNT);

#	ifdef DEBUG
	print_bytes(derived, CryptoPP::SHA512::DIGESTSIZE);
#	endif

	std::vector<uint8_t> seed;
	seed.reserve(CryptoPP::SHA512::DIGESTSIZE);
	for (CryptoPP::byte byte : derived) {
		seed.push_back(byte);
	}

	delete[] mnemonic;
	return seed;
}


} // namespace crypto
