#include <src/cli/color.h>

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
