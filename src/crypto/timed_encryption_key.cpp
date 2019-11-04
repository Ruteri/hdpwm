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

#include <src/crypto/timed_encryption_key.h>

#include <external/cryptopp/modes.h>
#include <external/cryptopp/osrng.h>
#include <external/cryptopp/pwdbased.h>
#include <external/cryptopp/sha.h>

#include <cassert>

namespace crypto {

void TimedEncryptionKey::encrypt(
    unsigned char *data_out, const unsigned char *data_in, size_t data_len) const {
	assert(this->valid);

	CryptoPP::byte iv[CryptoPP::AES::BLOCKSIZE];
	memset(iv, 0x00, CryptoPP::AES::BLOCKSIZE);

	CryptoPP::CFB_Mode<CryptoPP::AES>::Encryption cfbEncryption(
	    reinterpret_cast<const CryptoPP::byte *>(this->ec.data()), PasswordHash::Size, iv);
	cfbEncryption.ProcessData(data_out, data_in, data_len);
}

void TimedEncryptionKey::decrypt(
    unsigned char *data_out, const unsigned char *data_in, size_t data_len) const {
	assert(this->valid);

	CryptoPP::byte iv[CryptoPP::AES::BLOCKSIZE];
	memset(iv, 0x00, CryptoPP::AES::BLOCKSIZE);

	CryptoPP::CFB_Mode<CryptoPP::AES>::Decryption cfbDecryption(
	    reinterpret_cast<const CryptoPP::byte *>(this->ec.data()), PasswordHash::Size, iv);
	cfbDecryption.ProcessData(data_out, data_in, data_len);
}

} // namespace crypto
