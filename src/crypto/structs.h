#pragma once

#include <src/crypto/utils.h>

#include <array>
#include <charconv>
#include <cstdint>
#include <stdexcept>

namespace crypto {

template <int S, typename CV> struct ByteArray {
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
	decltype(auto) data() const { return this->_data.data(); }
	decltype(auto) size() const { return this->_data.size(); }

	decltype(auto) operator[](size_t index) { return this->_data[index]; }
	decltype(auto) operator[](size_t index) const { return this->_data[index]; }
};

struct PasswordHash : ByteArray<256, PasswordHash> {};
struct ChildDerivationData : ByteArray<296, ChildDerivationData> {};

struct Seed : ByteArray<512, Seed> {};
struct EncryptedSeed : ByteArray<512, EncryptedSeed> {};

template <typename AR> std::string serialize(const AR &data);

template <typename RARR> RARR deserialize(const std::string &hexstr);

} // namespace crypto
