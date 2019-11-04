#pragma once

#include <src/cli/utils.h>

#include <string>

/* forward declare as ncurses define OK which breaks leveldb */
struct _win_st;
typedef struct _win_st WINDOW;

class OutputHandler {
	Point origin;
	std::string output;

  public:
	OutputHandler(Point origin, std::string output) : origin(origin), output(std::move(output)) {}

	void draw();
	void draw(WINDOW *window);
};
