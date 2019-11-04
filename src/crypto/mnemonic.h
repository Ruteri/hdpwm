#pragma once

#include <src/crypto/structs.h>

#include <string>
#include <vector>

namespace crypto {

PasswordHash hash_password(const utils::sensitive_string &password);
EncryptedSeed encrypt_seed(Seed seed, PasswordHash password_hash);
Seed decrypt_seed(EncryptedSeed encrypted_seed, PasswordHash password_hash);

std::vector<std::string> generate_mnemonic(int entropy_size);
Seed mnemonic_to_seed(std::vector<std::string> words);

int find_word_index(std::string word);

} // namespace crypto
