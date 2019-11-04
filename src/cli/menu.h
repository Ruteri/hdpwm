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

#pragma once

#include <src/cli/utils.h>

#include <functional>
#include <string>
#include <vector>

/* forward declare as ncurses define OK which breaks leveldb */
struct _win_st;
typedef struct _win_st WINDOW;

struct BasicMenuEntry {
	std::string title;
	std::function<void()> on_accept;
};

class BasicMenu {
	size_t c_selected = 0;
	Point origin_pos;
	std::vector<BasicMenuEntry> links;

  public:
	BasicMenu(std::vector<BasicMenuEntry> links, Point pos = {0, 0});

	void draw();
	void draw(WINDOW *scr);
	void process_key(int key);
};
