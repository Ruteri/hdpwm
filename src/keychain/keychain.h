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

#include <src/keychain/db.h>
#include <src/keychain/keychain_entry.h>
#include <src/keychain/utils.h>

#include <src/crypto/structs.h>
#include <src/crypto/timed_encryption_key.h>

#include <filesystem>

namespace keychain {

class Keychain {
	// TODO: should be predefined and reserved m/0/0
	static constexpr crypto::DerivationPath standard_export_dpath{255};

  protected:
	std::filesystem::path data_path;

	std::unique_ptr<DB> db;
	crypto::TimedEncryptionKey tec;

  public:
	Keychain() = default;
	~Keychain() = default;

	Keychain(const Keychain &) = delete;
	Keychain &operator=(const Keychain &) = delete;

	Keychain(Keychain &&other);
	Keychain &operator=(Keychain &&other);

	static std::unique_ptr<Keychain> initialize_with_seed(
	    std::filesystem::path path, crypto::Seed seed, crypto::PasswordHash pw_hash);
	static std::unique_ptr<Keychain> open(std::filesystem::path path, crypto::PasswordHash pw_hash);

	void import_from_uri(const UriLocator &uri);
	void export_to_uri(const UriLocator &uri) const;

	std::string get_data_dir_path() const { return data_path.string(); }
	Directory::ptr get_root_dir() const;

	crypto::DerivationPath get_next_derivation_path();

	void save_entries(Directory::ptr root);

	static utils::sensitive_string encode_secret(
	    unsigned char *in_data, size_t in_size, size_t out_size);
	crypto::Seed derive_child(const crypto::DerivationPath &dpath) const;
	utils::sensitive_string derive_secret(const crypto::DerivationPath &dpath);
};

} // namespace keychain
