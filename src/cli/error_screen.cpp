#include <src/cli/error_screen.h>

#include <curses.h>

ErrorScreen::ErrorScreen(WindowManager *wmanager, Point origin, std::string msg) :
    ScreenController(wmanager), origin(origin), msg(std::move(msg)) {}

void ErrorScreen::m_draw() {
	clear();
	mvaddstr(origin.row, origin.col, "Encountered the following error:");
	mvaddstr(origin.row + 1, origin.col + 2, msg.c_str());
	mvaddstr(origin.row + 2, origin.col, "Press any key to continue.");
	noecho();
}

void ErrorScreen::m_on_key(int) { wmanager->pop_controller(); }
