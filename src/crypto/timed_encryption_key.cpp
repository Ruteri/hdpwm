#include <src/crypto/timed_encryption_key.h>

#include <external/cryptopp/modes.h>
#include <external/cryptopp/osrng.h>
#include <external/cryptopp/pwdbased.h>
#include <external/cryptopp/sha.h>

#include <cassert>

namespace crypto {

void TimedEncryptionKey::encrypt(unsigned char *data_out, unsigned char *data_in, size_t data_len) {
	assert(this->valid);

	CryptoPP::byte iv[ CryptoPP::AES::BLOCKSIZE ];
	memset( iv, 0x00, CryptoPP::AES::BLOCKSIZE );

	CryptoPP::CFB_Mode<CryptoPP::AES>::Encryption cfbEncryption(reinterpret_cast<CryptoPP::byte*>( this->ec.data() ), PasswordHash::Size, iv);
	cfbEncryption.ProcessData(data_out, data_in, data_len);
}

void TimedEncryptionKey::decrypt(unsigned char *data_out, unsigned char *data_in, size_t data_len) {
	assert(this->valid);

	CryptoPP::byte iv[ CryptoPP::AES::BLOCKSIZE ];
	memset( iv, 0x00, CryptoPP::AES::BLOCKSIZE );

	CryptoPP::CFB_Mode<CryptoPP::AES>::Decryption cfbDecryption(reinterpret_cast<CryptoPP::byte*>( this->ec.data() ), PasswordHash::Size, iv);
	cfbDecryption.ProcessData(data_out, data_in, data_len);
}

} // namespace crypto
