#pragma once

#include <string>
#include <vector>

namespace utils {

std::vector<std::string> split_string(std::string str);

std::vector<unsigned char> unhexify(std::string hexstr);

void print_bytes(void *buffer, int size);

} // namespace utils
