#pragma once

#include <cstddef>
#include <string>

namespace utils {

struct sensitive_string {
	size_t index = 0;
	size_t max_size = 32;
	char *data;

	sensitive_string();
	explicit sensitive_string(int size);
	sensitive_string(const sensitive_string &);
	sensitive_string(sensitive_string &&);

	sensitive_string &operator=(const sensitive_string &);
	sensitive_string &operator=(sensitive_string &&);

	explicit sensitive_string(const std::string &);
	explicit sensitive_string(std::string &&);

	~sensitive_string();

	size_t size() const;
	void resize(size_t);
	void reserve(size_t);

	void push_back(char c);
	void pop_back();
};

void secure_zero(void *s, size_t n);
void secure_zero_string(std::string &&s);

} // namespace utils
