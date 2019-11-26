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

#include <optional>
#include <string>
#include <vector>

namespace utils {

std::vector<std::string> split_string(std::string str);

void print_bytes(void *buffer, int size);

template <typename T = void> struct Result {
	std::optional<T> _value;
	std::string_view _reason;

	explicit operator bool() const { return static_cast<bool>(_value); }

	T value() const { return _value.value(); }
	auto what() { return _reason; }

	static Result Ok(T &&t) { return Result<T>{std::forward<T>(t), ""}; }
	static Result Err(std::string_view reason) { return Result{{}, reason}; }
};

template <> struct Result<void> {
	bool _value;
	std::string_view _reason;

	explicit operator bool() const { return _value; }

	auto what() { return _reason; }

	static Result Ok() { return Result{true, ""}; }
	static Result Err(std::string_view reason) { return Result{false, reason}; }
};

} // namespace utils
