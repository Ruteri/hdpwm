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

#include <src/cli/form_controller.h>

#include <curses.h>

FormController::FormController(WindowManager *wmanager, ScreenController *parent, WINDOW *&window,
    std::function<void()> on_done, std::function<void()> on_cancel) :
    ScreenController(wmanager),
    parent(parent), window(window), on_done(on_done), on_cancel(on_cancel) {
	m_cursor_prev_state = curs_set(2);
}

void FormController::m_init() {
	if (parent) parent->init();
	curs_set(2);
}

void FormController::m_cleanup() {
	curs_set(m_cursor_prev_state);
	if (parent) parent->cleanup();
}

void FormController::m_draw() {
	wclear(window);

	if (parent) parent->draw();

	if (fields.size() == 0) {
		curs_set(0);
	}

	for (auto &output : labels) {
		output->draw(window);
	}

	int cx = 0, cy = 0;
	for (size_t i = 0; i < fields.size(); ++i) {
		fields[i]->draw(window);
		if (i == current_input) getyx(window, cx, cy);
	}

	if (current_input <= fields.size() && state == State::PROCESSING) {
		wmove(window, cx, cy);
	}

	wrefresh(window);
}

void FormController::m_on_key(int key) {
	switch (state) {
	case State::IGNORING:
	case State::DONE:
		if (parent) parent->on_key(key);
		break;
	case State::PROCESSING:
		if (fields.empty()) {
			on_done();
			return;
		}

		switch (key) {
		case KEY_UP:
			current_input = current_input <= 0 ? 0 : current_input - 1;
			break;
		case KEY_DOWN:
		case '\t':
			fields[current_input]->process_key(KEY_ENTER);
			break;
		case KEY_ESC:
			on_cancel();
			break;
		default:
			fields[current_input]->process_key(key);
			break;
		}
	}
}

void FormController::advance_form() {
	if (++current_input >= fields.size()) {
		state = State::DONE;
		on_done();
	}
}

void FormController::add_label(std::string text) {
	Point origin{2 + static_cast<int>(fields.size() + labels.size()) * 3, 5};
	return add_label(origin, text);
}

void FormController::add_label(Point origin, std::string text) {
	labels.push_back(std::make_unique<StringOutputHandler>(std::move(origin), std::move(text)));
}

void FormController::add_output(std::unique_ptr<OutputHandler> output) {
	labels.push_back(std::move(output));
}
