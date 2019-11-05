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

#include <cstddef>
#include <string>

namespace utils {

struct sensitive_string {
	size_t index = 0;
	size_t max_size = 32;
	char *data = nullptr;

	sensitive_string();
	explicit sensitive_string(int size);
	sensitive_string(const sensitive_string &);
	sensitive_string(sensitive_string &&);

	sensitive_string &operator=(const sensitive_string &);
	sensitive_string &operator=(sensitive_string &&);

	sensitive_string(const char *);
	explicit sensitive_string(const std::string &);
	explicit sensitive_string(std::string &&);

	~sensitive_string();

	explicit operator std::string() const;

	size_t size() const;
	void resize(size_t);
	void reserve(size_t);

	void push_back(char c);
	void pop_back();
};

bool operator==(const sensitive_string &lhs, const sensitive_string &rhs);
bool operator!=(const sensitive_string &lhs, const sensitive_string &rhs);
bool operator<(const sensitive_string &lhs, const sensitive_string &rhs);

void secure_zero(void *s, size_t n);
void secure_zero_string(std::string &&s);

} // namespace utils
