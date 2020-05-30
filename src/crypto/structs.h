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

#include <src/crypto/utils.h>

#include <array>
#include <charconv>
#include <cstdint>
#include <stdexcept>
#include <vector>

namespace crypto {

template <int S, typename CV> struct ByteArray {
	static constexpr size_t Size = S / 8;
	using UnderlyingType = std::array<unsigned char, Size>;
	UnderlyingType _data;

	ByteArray() {
		lock_mem(_data.data(), Size);
	}

	~ByteArray() {
		unlock_mem(_data.data(), Size);
	}

	ByteArray(UnderlyingType &&other) : _data(std::move(other)) {}

	ByteArray(const ByteArray &other) : _data(other._data) {}
	ByteArray(ByteArray &&other) { this->_data = std::move(other._data); }

	ByteArray &operator=(const ByteArray &other) {
		this->_data = other._data;
		return *this;
	}

	ByteArray &operator=(ByteArray &&other) {
		this->_data = std::move(other._data);
		return *this;
	}

	bool operator==(const ByteArray &other) const { return this->_data == other._data; }

	decltype(auto) begin() const { return this->_data.begin(); }
	decltype(auto) end() const { return this->_data.end(); }

	decltype(auto) data() { return this->_data.data(); }
	decltype(auto) data() const { return this->_data.data(); }
	decltype(auto) size() const { return this->_data.size(); }

	decltype(auto) operator[](size_t index) { return this->_data[index]; }
	decltype(auto) operator[](size_t index) const { return this->_data[index]; }
};

struct PasswordHash : ByteArray<256, PasswordHash> {};
struct ChildDerivationData : ByteArray<296, ChildDerivationData> {};

struct Seed : ByteArray<512, Seed> {};
struct EncryptedSeed : ByteArray<512, EncryptedSeed> {};

struct EncryptionKey : ByteArray<256, EncryptionKey> {
	EncryptionKey() = default;
	EncryptionKey(const Seed &seed) : ByteArray<256, EncryptionKey>() { std::copy(seed.begin() + 32, seed.end(), _data.begin()); }
};

using Ciphertext = utils::sensitive_string;
using B64EncodedText = utils::sensitive_string;

template <typename AR> std::string serialize(const AR &data);

template <typename RARR> RARR deserialize(const std::string &hexstr);

} // namespace crypto
