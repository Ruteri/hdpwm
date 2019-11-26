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

Ciphertext encrypt(const EncryptionKey &key, const std::string &to_encrypt);
std::string decrypt(const EncryptionKey &key, const Ciphertext &to_decrypt);

EncryptedSeed encrypt_seed(const Seed &seed, const PasswordHash &password_hash);
Seed decrypt_seed(const EncryptedSeed &encrypted_seed, const PasswordHash &password_hash);
PasswordHash hash_password(const utils::sensitive_string &password);

Seed derive_child(const PasswordHash &pw_hash, const EncryptedSeed &encrypted_parent_key,
    const DerivationPath &path);

} // namespace crypto
