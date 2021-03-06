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
#include <src/tui/input.h>
#include <src/tui/manager.h>
#include <src/tui/menu.h>
#include <src/tui/output.h>
#include <src/tui/screen_controller.h>

#include <functional>
#include <memory>
#include <vector>

class FormController : public ScreenController {
  protected:
	ScreenController *parent;
	WINDOW *&window;
	std::function<void()> on_done;
	std::function<void()> on_cancel;

  private:
	enum class State { PROCESSING, IGNORING, DONE } state = State::PROCESSING;

	std::vector<std::unique_ptr<OutputHandler>> labels = {};

	// TODO: use unique_ptrs
	std::vector<InputHandler *> fields = {};
	size_t current_input = 0;

	/* parent has to maintain window and it's pointer */
	int m_cursor_prev_state;
	void m_init() override;
	void m_cleanup() override;

	void m_draw() override;
	void m_on_key(int key) override;

	void advance_form();

  public:
	FormController(WindowManager *wmanager);
	FormController(WindowManager *wmanager, ScreenController *parent, WINDOW *&window,
	    std::function<void()> on_done, std::function<void()> on_cancel);

	void add_label(std::string);
	void add_label(Point origin, std::string);

	void add_output(std::unique_ptr<OutputHandler>);

	template <typename InputType, typename... Args>
	InputType *add_field(
	    std::function<bool(const typename InputType::UValue &)> on_accept, Args &&... args) {
		InputType *input_handler = new InputType(
		    std::forward<Args>(args)..., [this, on_accept](const typename InputType::UValue &v) {
			    if (on_accept(v))
				    advance_form();
			    else {
			    }
		    });
		fields.push_back(input_handler);
		return input_handler;
	}
};
