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

#include <src/tui/color.h>

#include <curses.h>

void init_colors() {
	use_default_colors();
	start_color();
	init_pair(static_cast<int>(ColorPair::DEFAULT), -1, -1);
	init_pair(static_cast<int>(ColorPair::ALL_RED), COLOR_RED, COLOR_RED);
}

ColorGuard::ColorGuard(WINDOW *window, ColorPair color_pair) :
    window(window), color_pair(color_pair) {
	wattron(window, COLOR_PAIR(static_cast<int>(color_pair)));
}
ColorGuard::~ColorGuard() { wattroff(window, COLOR_PAIR(static_cast<int>(color_pair))); }
