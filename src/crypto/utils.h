#pragma once

#include <cstddef>
#include <string>

namespace utils {

struct sensitive_string {
	size_t index = 0;
	char *data = new char[256];

	sensitive_string();
	sensitive_string(sensitive_string &&);
	sensitive_string(const sensitive_string &) = delete;

	sensitive_string &operator=(sensitive_string &&);
	sensitive_string &operator=(const sensitive_string &) = delete;

	explicit sensitive_string(const std::string &);
	explicit sensitive_string(std::string &&);

	~sensitive_string();

	size_t size() const;

	void pop_back();

	// returns false if out of space
	bool push_back(char c);
};

void secure_zero(void *s, size_t n);
void secure_zero_string(std::string &s);

} // namespace utils
