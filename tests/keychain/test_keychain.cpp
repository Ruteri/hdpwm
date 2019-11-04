#include <src/keychain/keychain.h>

#include <external/catch2/catch.hpp>

struct seed_data_s {
	std::string seed;
	std::string expected;
	int size;
};

TEST_CASE( "secret encoding works as intended", "[keychain_secret_encoding]" ) {

	std::vector<seed_data_s> data = {
		{ "cb167a75be85dda7988b628dd9e5ffbe13d3acd86fcf4cc0e742de17d80b3e25fb9781acc11821301f15bb2728225093c9b302dca2e05239785952218c3da735", R"(MkAsM%uZXu)", 10 },
		{ "cb167a75be85dda7988b628dd9e5ffbe13d3acd86fcf4cc0e742de17d80b3e25fb9781acc11821301f15bb2728225093c9b302dca2e05239785952218c3da735", R"(MkAsM%uZXu*HKQTQj.g8~)", 21 },
		{ "cb167a75be85dda7988b628dd9e5ffbe13d3acd86fcf4cc0e742de17d80b3e25fb9781acc11821301f15bb2728225093c9b302dca2e05239785952218c3da735", R"(MkAsM%uZXu*HKQTQj.g8~b)", 22 },
		{ "cb167a75be85dda7988b628dd9e5ffbe13d3acd86fcf4cc0e742de17d80b3e25fb9781acc11821301f15bb2728225093c9b302dca2e05239785952218c3da735", R"(MkAsM%uZXu*HKQTQj.g8~b)", 23 },
	};

	for (auto &tc : data) {
		auto seed = crypto::deserialize<crypto::Seed>(tc.seed);
		auto encoded = Keychain::encode_secret(seed.data(), seed.size(), tc.size);
		REQUIRE( static_cast<std::string>(encoded) == tc.expected );
	}
}
