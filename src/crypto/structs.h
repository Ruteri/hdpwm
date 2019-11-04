#pragma once

#include <src/crypto/utils.h>

#include <array>
#include <cstdint>
#include <stdexcept>

namespace crypto {

template <int S> struct ByteArray {
	static constexpr size_t Size = S / 8;
	using UnderlyingType = std::array<unsigned char, Size>;
	UnderlyingType _data;

	ByteArray() = default;
	// ~ByteArray() {}

	ByteArray(UnderlyingType &&other) : _data(std::move(other)) {}

	ByteArray(const ByteArray &other) : _data(other._data) {}
	ByteArray(ByteArray &&other) { this->_data = std::move(other._data); }

	ByteArray &operator=(const ByteArray &&other) {
		this->_data = other._data;
		return *this;
	}

	ByteArray &operator=(ByteArray &&other) {
		this->_data = std::move(other._data);
		return *this;
	}

	bool operator==(const ByteArray &other) const { return this->_data == other._data; }

	decltype(auto) begin() { return this->_data.begin(); }
	decltype(auto) end() { return this->_data.end(); }

	decltype(auto) data() { return this->_data.data(); }
	decltype(auto) size() const { return this->_data.size(); }

	std::string serialize_to_string() const {
		std::string rs;
		rs.reserve(this->size());
		for (unsigned char c : this->_data) {
			rs.push_back((char)c);
		}

		return rs;
	}

	static ByteArray<S> deserialize_from_string(std::string str) {
		if (str.size() != S) {
			throw std::runtime_error(
			    "invalid string passed to deserialization (length does not match)");
		}

		ByteArray<S> rarr;
		for (int i = 0; i < S; ++i) {
			rarr[i] = str[i];
		}

		return rarr;
	}
};

struct PasswordHash : ByteArray<256> {};

struct Seed : ByteArray<512> {};
struct EncryptedSeed : Seed {};

} // namespace crypto
