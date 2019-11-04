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

std::vector<unsigned char> unhexify(std::string hexstr) {
	std::vector<unsigned char> rv;
	rv.reserve(hexstr.size() / 2);
	for (unsigned int i = 0; i + 1 < hexstr.size(); i += 2) {
		std::istringstream strm(hexstr.substr(i, 2));
		int v;
		strm >> std::hex >> v;
		rv.push_back(v);
	}

	return rv;
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
