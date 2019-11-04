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

#include <src/crypto/structs.h>

namespace crypto {

// TODO: should clear the encryption key if not used for <x> (e.g. 3 minutes)
// TODO: should use secret-service if available (via e.g. libsecret)
// else should use kernel key management if available (should be)
class TimedEncryptionKey {
	bool valid = false;
	PasswordHash ec;

  public:
	TimedEncryptionKey() = default;

	TimedEncryptionKey &operator=(const TimedEncryptionKey &other) = default;
	TimedEncryptionKey(const TimedEncryptionKey &other) = default;

	TimedEncryptionKey &operator=(TimedEncryptionKey &&other) = default;
	TimedEncryptionKey(TimedEncryptionKey &&other) = default;

	explicit TimedEncryptionKey(crypto::PasswordHash pw) : valid(true), ec(std::move(pw)) {}

	// TODO: should lock
	const PasswordHash &getPasswordHash() { return ec; }

	bool is_valid() const { return this->valid; }

	void encrypt(unsigned char *data_out, const unsigned char *data_in, size_t data_len) const;
	void decrypt(unsigned char *data_out, const unsigned char *data_in, size_t data_len) const;
};

} // namespace crypto
