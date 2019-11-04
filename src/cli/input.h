#pragma once

#include <src/cli/utils.h>

#include <src/crypto/utils.h>

#include <memory>
#include <functional>

class InputHandler {
	virtual void on_backspace() = 0;
	virtual void on_char(char c) = 0;

	virtual void on_accept() = 0;
	virtual void on_cancel() = 0;

public:
	virtual void draw() = 0;
	virtual void process_key(int key);
};

template <typename V>
class InputHandlerCallback: public InputHandler {
public:
	using Signal = std::function<void(V&)>;
	using UValue = V;

protected:
	V value{};

	Signal on_accept_cb;
	Signal on_cancel_cb;

	void on_accept() override { on_accept_cb(value); }
	void on_cancel() override { on_cancel_cb(value); };

public:
	InputHandlerCallback(Signal on_accept_cb, Signal on_cancel_cb): on_accept_cb(on_accept_cb), on_cancel_cb(on_cancel_cb) {}
};

class StringInputHandler: public InputHandlerCallback<std::string> {
	Point origin;
	std::string title;

	void on_backspace() override;
	void on_char(char c) override;

public:
	using Signal = typename InputHandlerCallback<std::string>::Signal;
	StringInputHandler(const Point& origin, const std::string& title, Signal on_accept, Signal on_cancel);

	void draw() override;
};

class SensitiveInputHandler: public InputHandlerCallback<utils::sensitive_string> {
	Point origin;
	std::string title;

	void on_backspace() override;
	void on_char(char c) override;

public:
	using Signal = typename InputHandlerCallback<utils::sensitive_string>::Signal;
	SensitiveInputHandler(const Point& origin, const std::string& title, Signal on_accept, Signal on_cancel);

	void draw() override;
};
