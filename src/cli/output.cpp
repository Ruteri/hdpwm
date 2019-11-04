#include <src/cli/output.h>

#include <curses.h>

void OutputHandler::draw() {
	move(origin.row, origin.col);
	clrtoeol();
	mvaddstr(origin.row, origin.col, output.c_str());
}
