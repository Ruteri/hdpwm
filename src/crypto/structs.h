#pragma once

#include <src/crypto/utils.h>

#include <array>
#include <cstdint>

namespace crypto {

template <int S>
struct MovableArray {
	static constexpr size_t Size = S/8;
	using UnderlyingType = std::array<unsigned char, Size>;
	UnderlyingType _data;

	MovableArray() = default;
	MovableArray(UnderlyingType&& other): _data(std::move(other)) {}

	MovableArray(const MovableArray&) = delete;
	MovableArray(MovableArray&& other) {
		this->_data = std::move(other._data);
	}

	MovableArray& operator=(const MovableArray&& other) = delete;
	MovableArray& operator=(MovableArray&& other) {
		this->_data = std::move(other._data);
		return *this;
	}

	bool operator==(const MovableArray& other) const {
		return this->_data == other._data;
	}

	decltype(auto) begin() { return _data.begin(); }
	decltype(auto) end() { return _data.end(); }

	decltype(auto) data() { return _data.data(); }
	decltype(auto) size() { return _data.size(); }
};

struct PasswordHash: MovableArray<256> {};

struct Seed: MovableArray<512> {};
struct EncryptedSeed: Seed {};

} // namespace crypto
