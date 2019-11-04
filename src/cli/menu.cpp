#include <src/cli/menu.h>

#include <src/cli/utils.h>

#include <curses.h>

#include <string>
#include <vector>

BasicMenu::BasicMenu(const Point pos, const std::vector<BasicMenuEntry>& links): origin_pos(pos), links(links) {}

void BasicMenu::output_menu_element(const Point& pos, BasicMenuEntry entry) {
	move(pos.row, 0);
	clrtoeol();
	mvaddstr(pos.row, pos.col, entry.title.c_str());
}

void BasicMenu::draw() {
	curs_set(0);

	int maxlines = LINES - 1;
	mvaddstr(maxlines, 0, "<right arrow> to accept | <up / down arrow> to change");

	for (size_t i = 0; i < links.size(); ++i) {
		if (i == this->c_selected) {
			attron(A_STANDOUT);
			output_menu_element({origin_pos.row + i, origin_pos.col}, this->links[i]);
			attroff(A_STANDOUT);
		} else {
			output_menu_element({origin_pos.row + i, origin_pos.col}, this->links[i]);
		}

	}
}

size_t BasicMenu::get_user_selection() {
	cbreak();
	noecho();
	keypad(stdscr, TRUE);

	for (;;) {
		this->draw();

		int ch = getch();
		switch(ch) {
		case KEY_DOWN:
			this->c_selected = (this->c_selected + 1) % this->links.size();
			break;
		case KEY_UP:
			this->c_selected = (this->links.size() + this->c_selected - 1) % this->links.size();
			break;
		case KEY_RIGHT: case KEY_ENTER: case KEY_RETURN:
			return this->c_selected;
		}
	}
}
