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

#include <src/tui/error_screen.h>

#include <curses.h>

ErrorScreen::ErrorScreen(WindowManager *wmanager, Point origin, std::string msg) :
    ScreenController(wmanager), origin(origin), msg(std::move(msg)) {}

void ErrorScreen::m_draw() {
	clear();
	mvaddstr(origin.row, origin.col, "Encountered the following error:");
	mvaddstr(origin.row + 1, origin.col + 2, msg.c_str());
	addstr(". Press any key to continue.");
	noecho();
}

void ErrorScreen::m_on_key(int) { wmanager->pop_controller(); }
