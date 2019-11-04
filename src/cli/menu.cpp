#include <src/cli/menu.h>

#include <src/cli/utils.h>

#include <curses.h>

#include <string>
#include <vector>

BasicMenu::BasicMenu(std::vector<BasicMenuEntry> links, Point pos): origin_pos(pos), links(std::move(links)) {}

void BasicMenu::draw() {
	return this->draw(stdscr);
}

void BasicMenu::draw(WINDOW *scr) {
	curs_set(0);

	int maxr, maxc;
	getmaxyx(scr, maxr, maxc);

	int max_entries = maxr - origin_pos.row - 1;
	int n_to_skip = std::max(0, static_cast<int>(this->c_selected) - max_entries + 1);
	for (int i = 0; i < std::min(max_entries, static_cast<int>(this->links.size())); ++i) {
		wmove(scr, origin_pos.row + i, 0);
		wclrtoeol(scr);
		std::string& to_print = links[i + n_to_skip].title;
		if (i + n_to_skip == this->c_selected) {
			wattron(scr, A_STANDOUT);
			mvwaddstr(scr, origin_pos.row + i, origin_pos.col, to_print.c_str());
			wattroff(scr, A_STANDOUT);
		} else {
			mvwaddstr(scr, origin_pos.row + i, origin_pos.col, to_print.c_str());
		}
	}
}

void BasicMenu::process_key(int key) {
	switch(key) {
	case KEY_DOWN:
		c_selected = (c_selected + 1) % links.size();
		break;
	case KEY_UP:
		c_selected = (links.size() + c_selected - 1) % links.size();
		break;
	case KEY_RIGHT: case KEY_ENTER: case KEY_RETURN:
		links[c_selected].on_accept();
		break;
	}
}
