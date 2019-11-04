#include <string>
#include <vector>

struct Point {
	int row;
	int col;
};

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

