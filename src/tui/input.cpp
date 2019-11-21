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

#include <src/tui/input.h>

#include <curses.h>

void InputHandler::draw() { this->m_draw(stdscr); }

void InputHandler::draw(WINDOW *window) { this->m_draw(window); }

void InputHandler::process_key(int key) {
	switch (key) {
	case KEY_ENTER:
	case KEY_RETURN:
		return on_accept();
	case KEY_BACKSPACE:
		this->on_backspace();
		break;
	default:
		if (key <= 0xff) {
			this->on_char(static_cast<char>(key));
		}
		break;
	}
}

StringInputHandler::StringInputHandler(
    const Point &origin, const std::string &title, ValueCallback on_accept) :
    InputHandlerCallback<std::string>(std::move(on_accept)),
    origin(origin), title(title) {}

void StringInputHandler::on_char(char c) { this->value.push_back(c); }

void StringInputHandler::on_backspace() {
	if (this->value.length() > 0) this->value.pop_back();
}

void StringInputHandler::m_draw(WINDOW *window) {
	wmove(window, this->origin.row, 0);
	wclrtoeol(window);

	mvwaddstr(window, this->origin.row, this->origin.col, this->title.c_str());
	waddstr(window, this->value.c_str());
}

SensitiveInputHandler::SensitiveInputHandler(
    const Point &origin, const std::string &title, ValueCallback on_accept) :
    InputHandlerCallback<utils::sensitive_string>(std::move(on_accept)),
    origin(origin), title(title) {}

void SensitiveInputHandler::on_char(char c) { this->value.push_back(c); }

void SensitiveInputHandler::on_backspace() {
	if (this->value.size() > 0) this->value.pop_back();
}

void SensitiveInputHandler::m_draw(WINDOW *window) {
	wmove(window, this->origin.row, 0);
	wclrtoeol(window);

	mvwaddstr(window, this->origin.row, this->origin.col, this->title.c_str());
	for (auto i = this->value.size(); i > 0; --i) {
		waddch(window, '*');
	}
}
