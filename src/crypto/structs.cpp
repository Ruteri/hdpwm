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

#include <src/crypto/structs.h>
#include <src/crypto/utils.h>

#include <iostream>

#include <array>
#include <charconv>
#include <cstdint>
#include <stdexcept>

namespace crypto {

template <typename AR> std::string serialize(const AR &data) {
	std::string rs;
	rs.reserve(data.size() * 2);

	for (unsigned int i = 0; i < data.size(); ++i) {
		std::array<char, 2> tmp_arr{'0', '0'};
		if (auto [p, ec] =
		        std::to_chars(tmp_arr.data(), tmp_arr.data() + tmp_arr.size(), (int)data[i], 16);
		    ec != std::errc()) {
			throw std::runtime_error("could not deserialize");
		}

		if (data[i] < 16) {
			rs.push_back('0');
			rs.push_back(tmp_arr[0]);
		} else {
			rs.push_back(tmp_arr[0]);
			rs.push_back(tmp_arr[1]);
		}
	}

	return rs;
}

template std::string serialize<Seed>(const Seed &);
template std::string serialize<Ciphertext>(const Ciphertext &);
template std::string serialize<EncryptedSeed>(const EncryptedSeed &);
template std::string serialize<EncryptionKey>(const EncryptionKey &);
template std::string serialize<PasswordHash>(const PasswordHash &);
template std::string serialize<ChildDerivationData>(const ChildDerivationData &);

template <typename RARR> RARR deserialize(const std::string &hexstr) {
	RARR rarr;
	auto hsize = hexstr.size();
	if constexpr (std::is_same<RARR, Ciphertext>::value) {
		if ((hsize & 0x1) != 0) {
			throw std::runtime_error(
			    "invalid string passed to deserialization (length is not even)");
		}

		rarr.resize(hsize / 2);
		rarr.index = hsize / 2;
	} else if (hsize != RARR::Size * 2) {
		throw std::runtime_error(
		    "invalid string passed to deserialization (length does not match)");
	}

	for (unsigned int i = 0; i + 2 <= hsize; i += 2) {
		if (auto [p, ec] =
		        std::from_chars(hexstr.c_str() + i, hexstr.c_str() + i + 2, (unsigned char &) rarr[i / 2], 16);
		    ec != std::errc()) {
			throw std::runtime_error(
			    "invalid string passed to deserialization (invalid character)");
		}
	}

	return rarr;
}

template Seed deserialize<Seed>(const std::string &);
template Ciphertext deserialize<Ciphertext>(const std::string &);
template EncryptedSeed deserialize<EncryptedSeed>(const std::string &);
template EncryptionKey deserialize<EncryptionKey>(const std::string &);
template PasswordHash deserialize<PasswordHash>(const std::string &);
template ChildDerivationData deserialize<ChildDerivationData>(const std::string &);

} // namespace crypto
