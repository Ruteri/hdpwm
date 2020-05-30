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

#include <src/crypto/utils.h>

#include <cstring>
#include <unistd.h>

#ifdef _POSIX_MEMLOCK_RANGE
#include <stdexcept>
#include <cerrno>
#include <sys/mman.h>
#endif

namespace crypto {

void lock_mem(const void *mem, size_t size) {
#ifdef _POSIX_MEMLOCK_RANGE
	if (int ret = mlock(mem, size); ret != 0) {
		throw std::runtime_error(std::string("could not lock memory: ") + std::strerror(errno));
	}
#endif
}

void unlock_mem(const void *mem, size_t size) {
#ifdef _POSIX_MEMLOCK_RANGE
	if (int ret = munlock(mem, size); ret != 0) {
		throw std::runtime_error(std::string("could not unlock memory: ") + std::strerror(errno));
	}
#endif
}

} // namespace crypto

namespace utils {

sensitive_string::sensitive_string() : sensitive_string(32) {}

sensitive_string::sensitive_string(int size) : index(0), max_size(size) {
	if (size == 0) return;

	_data = new char[size];
	crypto::lock_mem(_data, max_size);
}

sensitive_string::sensitive_string(sensitive_string &&other) {
	this->index = other.index;
	this->max_size = other.max_size;
	this->_data = other._data;
	other._data = nullptr;
	other.index = 0;
	other.max_size = 0;
}

sensitive_string &sensitive_string::operator=(sensitive_string &&other) {
	this->index = other.index;
	this->max_size = other.max_size;
	this->_data = other._data;
	other._data = nullptr;
	other.index = 0;
	other.max_size = 0;
	return *this;
}

sensitive_string::sensitive_string(const sensitive_string &other) {
	this->index = other.index;
	this->max_size = other.max_size;
	if (this->max_size == 0) return;

	this->_data = new char[this->max_size];
	crypto::lock_mem(this->_data, this->max_size);
	std::memcpy(this->_data, other._data, other.index);
}

sensitive_string &sensitive_string::operator=(const sensitive_string &other) {
	this->index = other.index;
	this->max_size = other.max_size;
	if (this->max_size == 0) return *this;

	this->_data = new char[max_size];
	crypto::lock_mem(this->_data, this->max_size);
	std::memcpy(this->_data, other._data, other.index);
	return *this;
}

sensitive_string::sensitive_string(const char *str) {
	size_t len = std::strlen(str);
	this->index = len;
	if (len == 0) return;
	this->resize(len);
	std::strncpy(this->_data, str, len);
}

sensitive_string::sensitive_string(const char *data, size_t size) {
	this->resize(size);
	std::memcpy(this->_data, data, size);
	this->index = size;
}

sensitive_string::sensitive_string(const std::string &str) {
	this->index = str.size();
	this->max_size = std::max(static_cast<size_t>(32), str.size());
	this->_data = new char[this->max_size];
	crypto::lock_mem(this->_data, this->max_size);
	std::memcpy(_data, str.c_str(), str.size());
}

sensitive_string::sensitive_string(std::string &&str) {
	this->index = str.size();
	this->max_size = std::max(static_cast<size_t>(32), str.size());
	// _data = str.data(); ?
	this->_data = new char[max_size];
	crypto::lock_mem(this->_data, this->max_size);
	std::memcpy(_data, str.c_str(), str.size());
	secure_zero(str.data(), str.size());
}

sensitive_string::~sensitive_string() {
	// delete _data
	if (!this->_data) return;
	secure_zero(this->_data, this->max_size);
	crypto::unlock_mem(_data, max_size);
	delete[] this->_data;
}

sensitive_string::operator std::string() const {
	std::string rs;
	const auto size = this->size();
	rs.reserve(size + 1); // additional space for null char
	for (auto i = 0; i < size; ++i) {
		rs.push_back(this->_data[i]);
	}
	rs[size] = '\0';

	return rs;
}

size_t sensitive_string::size() const { return this->index; }

void sensitive_string::resize(size_t new_size) {
	if (new_size == 0) return;

	index = std::min(index, new_size);
	char *new_ptr = new char[new_size];
	crypto::lock_mem(new_ptr, new_size);
	auto old_ptr = _data;
	auto old_size = max_size;
	if (old_ptr) {
		std::memcpy(new_ptr, old_ptr, index);
	}

	_data = new_ptr;
	max_size = new_size;

	if (old_ptr) {
		secure_zero(old_ptr, old_size);
		crypto::unlock_mem(old_ptr, old_size);
		delete[] old_ptr;
	}
}

void sensitive_string::reserve(size_t new_size) {
	if (new_size <= max_size || new_size == 0) return;
	return resize(new_size);
}

void sensitive_string::push_back(char c) {
	if (this->index >= max_size) {
		if (max_size == 0) {
			// possible use after move
			resize(32);
		} else {
			resize(max_size * 2);
		}
	}

	this->_data[this->index++] = c;
}

void sensitive_string::pop_back() {
	this->index = this->index > 0 ? this->index - 1 : 0;
	if (this->_data) {
		secure_zero(this->_data + this->index, 1);
	}
}

bool operator==(const sensitive_string &lhs, const sensitive_string &rhs) {
	if (lhs.index != rhs.index) return false;
	if (lhs.index == 0) return true;
	if (lhs._data == rhs._data) return true;
	return std::memcmp(lhs._data, rhs._data, rhs.index) == 0;
}

bool operator!=(const sensitive_string &lhs, const sensitive_string &rhs) { return !(lhs == rhs); }

bool operator<(const sensitive_string &lhs, const sensitive_string &rhs) {
	if (lhs._data == rhs._data) return false;
	int sc = std::strncmp(lhs._data, rhs._data, std::min(lhs.index, rhs.index));
	if (sc < 0) return true;
	if (sc == 0) return lhs.index < rhs.index;
	return false;
}

void secure_zero(void *s, size_t n) {
	volatile char *p = reinterpret_cast<char *>(s);
	while (n--) *p++ = 0;
}

void secure_zero_string(std::string &&s) { secure_zero(s.data(), s.size()); }

} // namespace utils
