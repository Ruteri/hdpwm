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
