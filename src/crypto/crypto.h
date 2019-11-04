#pragma once

#include <src/crypto/structs.h>

namespace crypto {

struct DerivationPath {
	int seed;
};

EncryptedSeed encrypt_seed(const Seed &seed, const PasswordHash &password_hash);
Seed decrypt_seed(const EncryptedSeed &encrypted_seed, const PasswordHash &password_hash);
PasswordHash hash_password(const utils::sensitive_string &password);

Seed derive_child(const PasswordHash &pw_hash, const EncryptedSeed &encrypted_parent_key,
    const DerivationPath &path);

} // namespace crypto
