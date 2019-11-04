#include <src/crypto/utils.h>

#include <cstring>

namespace utils {

sensitive_string::sensitive_string() : index(0), max_size(32), data(new char[max_size]) {}

sensitive_string::sensitive_string(int size) : index(0), max_size(size), data(new char[size]) {}

sensitive_string::sensitive_string(sensitive_string &&other) {
	this->index = other.index;
	this->max_size = other.max_size;
	this->data = other.data;
	other.data = nullptr;
	other.index = 0;
	other.max_size = 0;
}

sensitive_string &sensitive_string::operator=(sensitive_string &&other) {
	this->index = other.index;
	this->max_size = other.max_size;
	this->data = other.data;
	other.data = nullptr;
	other.index = 0;
	other.max_size = 0;
	return *this;
}

sensitive_string::sensitive_string(const sensitive_string &other) {
	this->index = other.index;
	this->max_size = other.max_size;
	this->data = new char[max_size];
	std::memcpy(this->data, other.data, other.index);
}

sensitive_string &sensitive_string::operator=(const sensitive_string &other) {
	this->index = other.index;
	this->max_size = other.max_size;
	this->data = new char[max_size];
	std::memcpy(this->data, other.data, other.index);
	return *this;
}

sensitive_string::sensitive_string(const std::string &str) {
	this->index = str.size();
	this->max_size = std::max(static_cast<size_t>(32), str.size());
	this->data = new char[max_size];
	std::memcpy(data, str.c_str(), str.size());
}

sensitive_string::sensitive_string(std::string &&str) {
	this->index = str.size();
	this->max_size = std::max(static_cast<size_t>(32), str.size());
	// data = str.data(); ?
	this->data = new char[max_size];
	std::memcpy(data, str.c_str(), str.size());
	secure_zero(str.data(), str.size());
}

sensitive_string::~sensitive_string() {
	// delete data
	if (!this->data) return;
	secure_zero(this->data, this->max_size);
	delete[] this->data;
}

sensitive_string::operator std::string() const {
	std::string rs;
	const auto size = this->size();
	rs.reserve(size + 1); // additional space for null char
	for (auto i = 0; i < size; ++i) {
		rs.push_back(this->data[i]);
	}
	rs[size] = '\0';

	return rs;
}

size_t sensitive_string::size() const { return this->index; }

void sensitive_string::resize(size_t new_size) {
	index = std::min(index, new_size - 1);
	char *new_ptr = new char[new_size];
	std::memcpy(new_ptr, data, index);

	secure_zero(data, max_size);

	max_size = new_size;
	delete[] data;
	data = new_ptr;
}

void sensitive_string::reserve(size_t new_size) {
	if (new_size <= max_size) return;
	return resize(new_size);
}

void sensitive_string::push_back(char c) {
	if (this->index >= max_size) {
		if (max_size == 0) {
			// possible use after move
			resize(32);
		} else {
			resize(max_size * 2);
		}
	}

	this->data[this->index++] = c;
}

void sensitive_string::pop_back() {
	this->index = this->index > 0 ? this->index - 1 : 0;
	secure_zero(this->data + this->index, 1);
}

void secure_zero(void *s, size_t n) {
	volatile char *p = reinterpret_cast<char *>(s);
	while (n--) *p++ = 0;
}

void secure_zero_string(std::string &&s) { secure_zero(s.data(), s.size()); }

} // namespace utils
