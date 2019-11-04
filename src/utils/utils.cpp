#include <src/utils/utils.h>

#include <iomanip>
#include <iostream>
#include <iterator>
#include <sstream>

namespace utils {

std::vector<std::string> split_string(std::string str) {
	std::istringstream str_iss(str);
	std::vector<std::string> split_str(
	    (std::istream_iterator<std::string>(str_iss)), std::istream_iterator<std::string>());
	return split_str;
}

void print_bytes(void *buffer, int size) {
	std::cout << "0x";
	for (int i = 0; i < size; ++i) {
		unsigned char v = (reinterpret_cast<unsigned char *>(buffer))[i];
		std::cout << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(v);
	}

	std::cout << std::endl;
}

} // namespace utils
