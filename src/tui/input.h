/*

Copyright (C) 2019 Mateusz Morusiewicz

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.

*/

#pragma once

#include <src/tui/fwd.h>
#include <src/tui/utils.h>

#include <src/crypto/utils.h>

#include <functional>
#include <memory>

class InputHandler {
	bool is_visible{true};
	virtual void m_draw(WINDOW *window) = 0;

  public:
	virtual void set_visible(bool v) final { is_visible = v; };
	virtual void draw() final;
	virtual void draw(WINDOW *window) final;

	virtual void process_key(int key) = 0;
};

/* Common non-templated interface for ValueInputHandler */
class BasicUserInputHandler : public InputHandler {
	virtual void on_accept() = 0;
	virtual void on_char(char) = 0;
	virtual void on_backspace() = 0;

  public:
	void process_key(int key) final;
};

template <typename V> class ValueInputHandler : public BasicUserInputHandler {
  public:
	using ValueCallback = std::function<void(const V &)>;
	using UValue = V;

  protected:
	V value{};
	ValueCallback on_accept_cb;

	void on_accept() final { on_accept_cb(value); }

  public:
	ValueInputHandler(ValueCallback on_accept_cb) : on_accept_cb(on_accept_cb) {}
};

class StringInputHandler : public ValueInputHandler<std::string> {
	Point origin;
	std::string title;

	void on_char(char c) override;
	void on_backspace() override;
	void m_draw(WINDOW *window) override;

  public:
	StringInputHandler(const Point &origin, const std::string &title, ValueCallback on_accept);

	// allows setting displayed value
	void set_value(const std::string &v) { value = v; }
};

class SensitiveInputHandler : public ValueInputHandler<utils::sensitive_string> {
	Point origin;
	std::string title;

	void on_char(char c) override;
	void on_backspace() override;
	void m_draw(WINDOW *window) override;

  public:
	SensitiveInputHandler(const Point &origin, const std::string &title, ValueCallback on_accept);
};
