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

#pragma once

#include <string>
#include <vector>

namespace utils {

std::vector<std::string> split_string(std::string str);

void print_bytes(void *buffer, int size);

struct Result {
	bool valid;
	std::string_view reason;

	static Result Ok() { return Result{true, ""}; }
	static Result Err(std::string_view reason) { return Result{false, reason}; }

	explicit operator bool() const { return valid; }
};

} // namespace utils
