#include <src/crypto/mnemonic.h>
#include <src/crypto/utils.h>
#include <src/utils/utils.h>

#include <vector>

struct mnemonic_data {
	std::vector<std::string> mnemonic;
	crypto::Seed::UnderlyingType expected_seed;

	mnemonic_data(std::string unparsed_mnemonic, std::string unparsed_seed) {
		this->mnemonic = utils::split_string(unparsed_mnemonic);
		auto bytev = utils::unhexify(unparsed_seed);
		std::copy(bytev.begin(), bytev.end(), this->expected_seed.begin());
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
