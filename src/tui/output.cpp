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

#include <src/tui/output.h>

#include <src/tui/color.h>

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
