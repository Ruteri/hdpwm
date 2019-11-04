#pragma once

#include <src/cli/screens.h>
#include <src/cli/utils.h>

#include <src/crypto/utils.h>

#include <memory>
#include <functional>

enum class input_action_result { BACK, CONTINUE };

class InputHandler {
public:
	virtual std::unique_ptr<Screen> on_back() { return nullptr; }
	virtual std::unique_ptr<Screen> on_continue() { return nullptr; }
	virtual void on_backspace() = 0;
	virtual void on_char(int &c) = 0;

	virtual void draw() = 0;

	virtual input_action_result process();
};

class StringInputHandler: public InputHandler {
public:
	Point origin;
	std::string title;
	std::string value = "";

	StringInputHandler(const Point& origin, const std::string& title);

	void on_backspace() override;

	void on_char(int &c) override;

	void draw() override;
};

class SensitiveInputHandler: public InputHandler {
public:
	Point origin;
	std::string title;
	utils::sensitive_string value;

	SensitiveInputHandler(const Point& origin, const std::string& title);

	void on_backspace() override;

	void on_char(int &c) override;

	void draw() override;
};
