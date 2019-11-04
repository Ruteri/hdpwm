#include <src/crypto/utils.h>

namespace utils {

sensitive_string::sensitive_string() {}

sensitive_string::sensitive_string(sensitive_string&& other) {
	this->value = other.value;
	other.value = nullptr;
}

sensitive_string& sensitive_string::operator=(sensitive_string&& other) {
	this->value = other.value;
	other.value = nullptr;
	return *this;
}

sensitive_string::~sensitive_string() {
	// delete data
	if (!this->value) return;
	secure_zero(this->value, this->index);
	delete[] this->value;
}

void sensitive_string::pop_back() {
	this->value[this->index] = '\0';
	this->index = this->index > 0 ? this->index - 1 : 0;
}

// returns false if out of space
bool sensitive_string::push_back(char c) {
	if (this->index < 255) {
		this->value[++this->index] = c;
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
