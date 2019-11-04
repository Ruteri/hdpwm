#pragma once

#include <src/cli/fwd.h>

void init_colors();

enum class ColorPair : int { DEFAULT = 0, ALL_RED = 1 };

struct ColorGuard {
	WINDOW *window;
	ColorPair color_pair;

	ColorGuard(WINDOW *window, ColorPair color_pair);
	~ColorGuard();
};
