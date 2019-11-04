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
