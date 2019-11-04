#include <src/cli/output.h>

#include <src/cli/color.h>

#include <curses.h>

void OutputHandler::draw() { m_draw(stdscr); }

void OutputHandler::draw(WINDOW *window) { m_draw(window); }

void StringOutputHandler::m_draw(WINDOW *window) {
	wmove(window, origin.row, origin.col);
	wclrtoeol(window);
	mvwaddstr(window, origin.row, origin.col, output.c_str());
}

void SensitiveOutputHandler::m_draw(WINDOW *window) {
	wmove(window, origin.row, origin.col);
	wclrtoeol(window);

	ColorGuard cg{window, ColorPair::ALL_RED};
	for (size_t i = 0; i < sensitive_output.size(); ++i) {
		waddch(window, sensitive_output.data[i]);
	}
}
