#pragma once

#include <string>
#include <vector>
#include <cstdint>

namespace crypto {

std::vector<std::string> generate_mnemonic(int entropy_size);

std::vector<uint8_t> mnemonic_to_seed(std::vector<std::string> words);

int find_word_index(std::string word);

} // namespace crypto
