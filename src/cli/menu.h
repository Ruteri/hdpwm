#pragma once

#include <src/cli/utils.h>

#include <string>
#include <vector>

struct BasicMenuEntry {
	std::string title;
};

class BasicMenu {
	size_t c_selected = 0;
	Point origin_pos;
	std::vector<BasicMenuEntry> links;

	void output_menu_element(const Point& p, BasicMenuEntry entry);

public:
	BasicMenu(const Point pos, const std::vector<BasicMenuEntry>& links);

	void draw();

	size_t get_user_selection();
};

