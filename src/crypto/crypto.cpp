#include <src/crypto/crypto.h>

#include <external/cryptopp/modes.h>
#include <external/cryptopp/osrng.h>
#include <external/cryptopp/sha.h>

// constexpr static std::array<unsigned int, 256> secp256k1_n{0xFF, 0xFF 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE, 0xBA, 0xAE, 0xDC, 0xE6, 0xAF, 0x48, 0xA0, 0x3B, 0xBF, 0xD2, 0x5E, 0x8C, 0xD0, 0x36, 0x41, 0x41};

namespace crypto {

PasswordHash hash_password(const utils::sensitive_string &password) {
	PasswordHash pw_hash;

	CryptoPP::SHA256 sha;
	sha.CalculateDigest(
	    pw_hash.data(), reinterpret_cast<CryptoPP::byte *>(password.data), password.size());

	return pw_hash;
}

EncryptedSeed encrypt_seed(const Seed &seed, const PasswordHash &password_hash) {
	EncryptedSeed encrypted_seed;

	CryptoPP::byte iv[CryptoPP::AES::BLOCKSIZE];
	memset(iv, 0x00, CryptoPP::AES::BLOCKSIZE);

	CryptoPP::CFB_Mode<CryptoPP::AES>::Encryption cfbEncryption(
	    reinterpret_cast<const CryptoPP::byte *>(password_hash.data()), PasswordHash::Size, iv);
	cfbEncryption.ProcessData(reinterpret_cast<CryptoPP::byte *>(encrypted_seed.data()),
	    reinterpret_cast<const CryptoPP::byte *>(seed.data()), Seed::Size);

	return encrypted_seed;
}

Seed decrypt_seed(const EncryptedSeed &encrypted_seed, const PasswordHash &password_hash) {
	Seed seed;

	CryptoPP::byte iv[CryptoPP::AES::BLOCKSIZE];
	memset(iv, 0x00, CryptoPP::AES::BLOCKSIZE);

	CryptoPP::CFB_Mode<CryptoPP::AES>::Decryption cfbDecryption(
	    reinterpret_cast<const CryptoPP::byte *>(password_hash.data()), PasswordHash::Size, iv);
	cfbDecryption.ProcessData(reinterpret_cast<CryptoPP::byte *>(seed.data()),
	    reinterpret_cast<const CryptoPP::byte *>(encrypted_seed.data()), EncryptedSeed::Size);

	return seed;
}

Seed derive_child(const PasswordHash &pw_hash, const EncryptedSeed &encrypted_parent_key,
    const DerivationPath &path) {
	Seed decrypted_parent_key = decrypt_seed(encrypted_parent_key, pw_hash);

	ChildDerivationData cdd;
	cdd[0] = 0x00;
	std::strncpy(reinterpret_cast<char *>(cdd.data() + 1),
	    reinterpret_cast<const char *>(decrypted_parent_key.data() + 32), 32);
	cdd[33] = 0x01;
	cdd[34] = 0x00;
	cdd[35] = path.seed >> 8;
	cdd[36] = path.seed & 0xff;

	CryptoPP::SHA512 sha;
	sha.Update(reinterpret_cast<CryptoPP::byte *>(decrypted_parent_key.data()), 32);
	sha.Update(reinterpret_cast<CryptoPP::byte *>(cdd.data()), cdd.size());

	Seed derived_seed;
	sha.Final(derived_seed.data());

	// TODO: to be compliant with BIP32 should calculate child mod secp256k1_n

	return derived_seed;
}

} // namespace crypto
