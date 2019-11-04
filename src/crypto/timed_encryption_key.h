#pragma once

#include <src/crypto/structs.h>

namespace crypto {

// TODO: should clear the encryption key if not used for <x> (e.g. 3 minutes)
class TimedEncryptionKey {
	bool valid = false;
	crypto::PasswordHash ec;

public:
	TimedEncryptionKey() = default;
	TimedEncryptionKey(crypto::PasswordHash&& pw): valid(true), ec(std::move(pw)) {}

	bool is_valid() const { return this->valid; }

	void encrypt(unsigned char *data_out, unsigned char *data_in, size_t data_len);
	void decrypt(unsigned char *data_out, unsigned char *data_in, size_t data_len);
};

} // namespace crypto
