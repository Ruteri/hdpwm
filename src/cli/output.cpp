#include <src/cli/output.h>

#include <curses.h>

void OutputHandler::draw() {
	draw(stdscr);
}

void OutputHandler::draw(WINDOW *window) {
	wmove(window, origin.row, origin.col);
	wclrtoeol(window);
	mvwaddstr(window, origin.row, origin.col, output.c_str());
}
