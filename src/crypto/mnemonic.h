#pragma once

#include <src/crypto/structs.h>

#include <string>
#include <vector>

namespace crypto {

std::vector<std::string> generate_mnemonic(int entropy_size);
Seed mnemonic_to_seed(std::vector<std::string> words);

int find_word_index(std::string word);

} // namespace crypto
