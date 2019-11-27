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

#pragma once

#include <src/crypto/structs.h>

namespace crypto {

struct DerivationPath {
	int seed;
};

using B64EncodedText = std::vector<uint8_t>;
B64EncodedText as_encoded(const std::string &encoded_text);
Ciphertext as_ciphertext(const std::string &encoded_text);
std::string as_string(const B64EncodedText &encoded_text);

B64EncodedText base64_encode(const std::string &to_encode);
B64EncodedText base64_encode(const Ciphertext &to_encode);
std::string base64_decode(const B64EncodedText &to_decode);

Ciphertext encrypt(const EncryptionKey &key, const B64EncodedText &to_encrypt);
B64EncodedText decrypt(const EncryptionKey &key, const Ciphertext &to_decrypt);

EncryptedSeed encrypt_seed(const Seed &seed, const PasswordHash &password_hash);
Seed decrypt_seed(const EncryptedSeed &encrypted_seed, const PasswordHash &password_hash);
PasswordHash hash_password(const utils::sensitive_string &password);

Seed derive_child(const PasswordHash &pw_hash, const EncryptedSeed &encrypted_parent_key,
    const DerivationPath &path);

} // namespace crypto
