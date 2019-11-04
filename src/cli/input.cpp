#include <src/cli/input.h>

#include <curses.h>

void InputHandler::process_key(int key) {
	switch (key) {
	case KEY_SLEFT: case ERR:
		return on_cancel();
	case KEY_ENTER: case KEY_RETURN:
		return on_accept();
	case KEY_BACKSPACE:
		this->on_backspace();
		break;
	default:
		this->on_char((char) key);
		break;
	}
}

StringInputHandler::StringInputHandler(const Point& origin, const std::string& title, Signal on_accept, Signal on_cancel): InputHandlerCallback<std::string>(std::move(on_accept), std::move(on_cancel)), origin(origin), title(title) {}

void StringInputHandler::on_backspace() {
	if (this->value.length() > 0) this->value.pop_back();
}

void StringInputHandler::on_char(char c) {
	this->value.push_back(c);
}

void StringInputHandler::draw() {
	move(this->origin.row, 0);
	clrtoeol();

	mvaddstr(this->origin.row, this->origin.col, this->title.c_str());
	addstr(this->value.c_str());
}

SensitiveInputHandler::SensitiveInputHandler(const Point& origin, const std::string& title, Signal on_accept, Signal on_cancel): InputHandlerCallback<utils::sensitive_string>(std::move(on_accept), std::move(on_cancel)), origin(origin), title(title) {}

void SensitiveInputHandler::on_backspace() {
	if (this->value.size() > 0) this->value.pop_back();
}

void SensitiveInputHandler::on_char(char c) {
	this->value.push_back(c);
}

void SensitiveInputHandler::draw() {
	move(this->origin.row, 0);
	clrtoeol();

	mvaddstr(this->origin.row, this->origin.col, this->title.c_str());
	for (auto i = this->value.size(); i > 0; --i) {
		addch('*');
	}
}
