#pragma once

constexpr int KEY_RETURN = 10;
constexpr int KEY_RAW_ALT = 27;
constexpr int KEY_ESC = 27; // https://stackoverflow.com/questions/5977395/ncurses-and-esc-alt-keys

struct Point {
	int row;
	int col;
};
