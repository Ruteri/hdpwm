#include <src/crypto/mnemonic.h>

#include <src/crypto/mnemonic-wordlist.cpp>
#include <src/utils/utils.h>

#include <external/cryptopp/modes.h>
#include <external/cryptopp/osrng.h>
#include <external/cryptopp/pwdbased.h>
#include <external/cryptopp/sha.h>

#include <stdexcept>

namespace crypto {

constexpr int PBKDF2_ITERATION_COUNT = 2048;

PasswordHash hash_password(utils::sensitive_string&& password) {
	PasswordHash pw_hash;

	CryptoPP::SHA256 sha;
	sha.CalculateDigest(pw_hash.data(), reinterpret_cast<CryptoPP::byte*>(password.data), password.size());

	return pw_hash;
}

EncryptedSeed encrypt_seed(Seed seed, PasswordHash password_hash) {
	EncryptedSeed encrypted_seed;

	CryptoPP::byte iv[ CryptoPP::AES::BLOCKSIZE ];
	memset( iv, 0x00, CryptoPP::AES::BLOCKSIZE );

	CryptoPP::CFB_Mode<CryptoPP::AES>::Encryption cfbEncryption(reinterpret_cast<CryptoPP::byte*>( password_hash.data() ), PasswordHash::Size, iv);
	cfbEncryption.ProcessData(
	    reinterpret_cast<CryptoPP::byte*>(encrypted_seed.data()),
	    reinterpret_cast<CryptoPP::byte*>(seed.data()), Seed::Size);

	return encrypted_seed;
}

Seed decrypt_seed(EncryptedSeed encrypted_seed, PasswordHash password_hash) {
	Seed seed;

	CryptoPP::byte iv[ CryptoPP::AES::BLOCKSIZE ];
	memset( iv, 0x00, CryptoPP::AES::BLOCKSIZE );

	CryptoPP::CFB_Mode<CryptoPP::AES>::Decryption cfbDecryption(reinterpret_cast<CryptoPP::byte*>( password_hash.data() ), PasswordHash::Size, iv);
	cfbDecryption.ProcessData(
	    reinterpret_cast<CryptoPP::byte*>(seed.data()),
	    reinterpret_cast<CryptoPP::byte*>(encrypted_seed.data()), EncryptedSeed::Size);

	return seed;
}

uint8_t extract_bit(const CryptoPP::byte *buffer, int cb) {
	const CryptoPP::byte byte = buffer[cb/8];
	const uint8_t mask = 1 << (7 - (cb % 8));
	return byte & mask;
}

std::string word_at(int index) {
	return mnemonic_dictionary[index % mnemonic_dictionary.size()];
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

std::string get_random_word() {
	CryptoPP::byte seed[2];
	CryptoPP::NonblockingRng rng;
	rng.GenerateBlock(seed, sizeof(seed));
	uint16_t random_index = seed[0] << 8 | seed[1];
	return word_at(random_index);
}

std::vector<std::string> generate_mnemonic(int entropy_size) {

	constexpr int CHECKSUM_MAX_SIZE = 1;

	CryptoPP::byte *seed = new CryptoPP::byte[entropy_size + CHECKSUM_MAX_SIZE];

	CryptoPP::NonblockingRng rng;
	rng.GenerateBlock(seed, entropy_size);

	CryptoPP::SHA256 checksum_digester;
	checksum_digester.CalculateTruncatedDigest(seed + entropy_size, 1, seed, entropy_size);

	auto indices = bitsplit_11(seed, entropy_size * 8 + entropy_size / 4);
	auto words = get_words_from_indices(indices);

	std::string extra_word = get_random_word();
	words.push_back(extra_word);

	delete[] seed;
	return words;
}

Seed mnemonic_to_seed(std::vector<std::string> words) {
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

	const CryptoPP::byte salt[] = "ob1Ofabex?reg+ojAfKosh89OkEgUsvojbeurOv7knok";

	Seed seed;
	CryptoPP::PKCS5_PBKDF2_HMAC<CryptoPP::SHA512> pbkdf;
	CryptoPP::byte unused = 0;

	pbkdf.DeriveKey(reinterpret_cast<CryptoPP::byte*>(seed.data()), CryptoPP::SHA512::DIGESTSIZE, unused, mnemonic, words_len, salt, sizeof(salt), PBKDF2_ITERATION_COUNT);

	delete[] mnemonic;
	return seed;
}

} // namespace crypto
