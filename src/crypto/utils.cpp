#include <src/crypto/utils.h>

namespace utils {

sensitive_string::sensitive_string() {}

sensitive_string::sensitive_string(sensitive_string&& other) {
	this->data = other.data;
	other.data = nullptr;
}

sensitive_string& sensitive_string::operator=(sensitive_string&& other) {
	this->data = other.data;
	other.data = nullptr;
	return *this;
}

sensitive_string::sensitive_string(const std::string& str) {
	for (char c : str) {
		this->push_back(c);
	}
}

sensitive_string::sensitive_string(std::string&& str) {
	// Might be optimized
	for (char c : str) {
		this->push_back(c);
	}

	secure_zero_string(str);
}

sensitive_string::~sensitive_string() {
	// delete data
	if (!this->data) return;
	secure_zero(this->data, this->index);
	delete[] this->data;
}

size_t sensitive_string::size() const {
	return this->index;
}

void sensitive_string::pop_back() {
	this->data[this->index] = '\0';
	this->index = this->index > 0 ? this->index - 1 : 0;
}

// returns false if out of space
bool sensitive_string::push_back(char c) {
	if (this->index < 255) {
		this->data[++this->index] = c;
		return true;
	}

	return false;
}

void secure_zero(void *s, size_t n) {
    volatile char *p = reinterpret_cast<char*>(s);
    while (n--) *p++ = 0;
}

void secure_zero_string(std::string& s) {
	secure_zero(s.data(), s.size());
}

} // namespace utils
