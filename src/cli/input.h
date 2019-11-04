#pragma once

#include <src/cli/fwd.h>
#include <src/cli/utils.h>

#include <src/crypto/utils.h>

#include <functional>
#include <memory>

class InputHandler {
	virtual void on_accept() = 0;

	virtual void on_char(char c) = 0;
	virtual void on_backspace() = 0;
	virtual void m_draw(WINDOW *window) = 0;

  public:
	void draw();
	void draw(WINDOW *window);

	virtual void process_key(int key);
};

template <typename V> class InputHandlerCallback : public InputHandler {
  public:
	using ValueCallback = std::function<void(const V &)>;
	using UValue = V;

  protected:
	V value{};

	ValueCallback on_accept_cb;

	void on_accept() override { on_accept_cb(value); }

  public:
	InputHandlerCallback(ValueCallback on_accept_cb) : on_accept_cb(on_accept_cb) {}
};

class StringInputHandler : public InputHandlerCallback<std::string> {
	Point origin;
	std::string title;

	void on_char(char c) override;
	void on_backspace() override;
	void m_draw(WINDOW *window) override;

  public:
	using ValueCallback = typename InputHandlerCallback<std::string>::ValueCallback;
	StringInputHandler(const Point &origin, const std::string &title, ValueCallback on_accept);

	// allows setting displayed value
	void set_value(const std::string &v) { value = v; }
};

class SensitiveInputHandler : public InputHandlerCallback<utils::sensitive_string> {
	Point origin;
	std::string title;

	void on_char(char c) override;
	void on_backspace() override;
	void m_draw(WINDOW *window) override;

  public:
	using ValueCallback = typename InputHandlerCallback<utils::sensitive_string>::ValueCallback;
	SensitiveInputHandler(const Point &origin, const std::string &title, ValueCallback on_accept);
};
