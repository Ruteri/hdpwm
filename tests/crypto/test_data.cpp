#include <src/utils/utils.h>

#include <vector>

struct mnemonic_data {
	std::vector<std::string> mnemonic;
	std::vector<uint8_t> expected_seed;

	mnemonic_data(std::string unparsed_mnemonic, std::string unparsed_seed) {
		this->mnemonic = utils::split_string(unparsed_mnemonic);
		this->expected_seed = utils::unhexify(unparsed_seed);
	}
};

std::vector<mnemonic_data> mnemonic_test_vector = {

	{
		"abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon about",
		"b9ab11d9d9358453e13c18e1a3ec483433e1a6acc921cbdad87b1782103af85ade2530dfd28e1f18dcd8552a7fa80996e33daf34b678ae52696330a89970cf84"
		// todo: adjust to match bip39 test vector
	},
};

std::vector<std::vector<std::string>> invalid_mnemonic_test_vector = {
	utils::split_string("abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon xxx"),
};
