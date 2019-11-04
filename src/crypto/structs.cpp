#include <src/crypto/structs.h>

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
template std::string serialize<EncryptedSeed>(const EncryptedSeed &);
template std::string serialize<PasswordHash>(const PasswordHash &);
template std::string serialize<ChildDerivationData>(const ChildDerivationData &);

template <typename RARR> RARR deserialize(const std::string &hexstr) {
	if (hexstr.size() != RARR::Size * 2) {
		throw std::runtime_error(
		    "invalid string passed to deserialization (length does not match)");
	}

	RARR rarr;
	for (unsigned int i = 0; i + 2 <= hexstr.size(); i += 2) {
		if (auto [p, ec] =
		        std::from_chars(hexstr.c_str() + i, hexstr.c_str() + i + 2, rarr[i / 2], 16);
		    ec != std::errc()) {
			throw std::runtime_error(
			    "invalid string passed to deserialization (invalid character)");
		}
	}

	return rarr;
}

template Seed deserialize<Seed>(const std::string &);
template EncryptedSeed deserialize<EncryptedSeed>(const std::string &);
template PasswordHash deserialize<PasswordHash>(const std::string &);
template ChildDerivationData deserialize<ChildDerivationData>(const std::string &);

} // namespace crypto
