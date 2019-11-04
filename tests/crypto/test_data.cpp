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

#include <src/crypto/mnemonic.h>
#include <src/crypto/utils.h>
#include <src/utils/utils.h>

#include <vector>

struct mnemonic_data {
	std::vector<std::string> mnemonic;
	crypto::Seed expected_seed{};

	mnemonic_data(std::string unparsed_mnemonic, std::string unparsed_seed) {
		this->mnemonic = utils::split_string(unparsed_mnemonic);
		if (!unparsed_seed.empty()) {
			expected_seed = crypto::deserialize<crypto::Seed>(unparsed_seed);
		}
	}
};

std::vector<mnemonic_data> mnemonic_test_vector = {

	{
		"abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon about",
		"f29b278b6525f2bf2e78c24ed086d58836438509bea3d837e1434ee6d3b082c23e60a199c9eb05dc1f6307bb99aca5025e2241fec580312b0064b375020cc2fd",
		// todo: adjust to match bip39 test vector
	},
};

std::vector<mnemonic_data> invalid_mnemonic_test_vector = {
	{
		"abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon xxx",
		"",
	},
};
