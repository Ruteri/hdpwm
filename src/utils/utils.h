#pragma once

#include <vector>
#include <sstream>
#include <iterator>

namespace utils {

std::vector<std::string> split_string(std::string str) {
	std::istringstream str_iss(str);
	std::vector<std::string> split_str((std::istream_iterator<std::string>(str_iss)),
							 std::istream_iterator<std::string>());
	return split_str;
}

std::vector<uint8_t> unhexify(std::string hexstr) {
	std::vector<uint8_t> rv;
	rv.reserve(hexstr.size()/2);
	for (unsigned int i = 0; i < hexstr.size() - 1; i += 2) {
		std::istringstream strm(hexstr.substr(i, 2));
		int v;
		strm >> std::hex >> v;
		rv.push_back(v);
	}

	return rv;
}

} // namespace utils
