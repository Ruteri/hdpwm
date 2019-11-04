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
