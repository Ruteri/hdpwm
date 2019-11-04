#pragma once

#include <string>
#include <vector>

namespace utils {

std::vector<std::string> split_string(std::string str);

void print_bytes(void *buffer, int size);

} // namespace utils
