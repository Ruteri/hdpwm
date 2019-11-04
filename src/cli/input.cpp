#include <src/cli/input.h>

#include <curses.h>

input_action_result InputHandler::process() {
	cbreak();
	echo();
	curs_set(2);

	for (;;) {
		this->draw();

		int ch = getch();
		switch (ch) {
		case KEY_SLEFT: case ERR:
			return input_action_result::BACK;
		case KEY_ENTER: case KEY_RETURN:
			return input_action_result::CONTINUE;
		case KEY_BACKSPACE:
			this->on_backspace();
			break;
		default:
			this->on_char(ch);
			break;
		}
		utils::secure_zero(&ch, 1);
	}
}

StringInputHandler::StringInputHandler(const Point& origin, const std::string& title): origin(origin), title(title) {}

void StringInputHandler::on_backspace() {
	this->value.pop_back();
}

void StringInputHandler::on_char(int &c) {
	this->value.push_back((char) c);
}

void StringInputHandler::draw() {
	move(this->origin.row, 0);
	clrtoeol();

	mvaddstr(this->origin.row, this->origin.col, this->title.c_str());
	addstr(this->value.c_str());
}

SensitiveInputHandler::SensitiveInputHandler(const Point& origin, const std::string& title): origin(origin), title(title) {}

void SensitiveInputHandler::on_backspace() {
	this->value.pop_back();
}

void SensitiveInputHandler::on_char(int &c) {
	char ch = c;
	this->value.push_back(ch);
	utils::secure_zero(&c, 1);
	utils::secure_zero(&ch, 1);
}

void SensitiveInputHandler::draw() {
	move(this->origin.row, 0);
	clrtoeol();

	mvaddstr(this->origin.row, this->origin.col, this->title.c_str());
	for (auto i = this->value.size(); i > 0; --i) {
		addch('*');
	}
}
