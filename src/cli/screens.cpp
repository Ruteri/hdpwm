#include <src/cli/screens.h>

#include <src/cli/menu.h>

#include <curses.h>

#include <functional>
#include <vector>

std::unique_ptr<Screen> StartScreen::run() {
	clear();

	mvaddstr(0, 0, "Deterministic password manager");

	std::vector<BasicMenuEntry> start_screen_menu_entries = {
		{ "Import keychain" },
		{ "Create new keychain" },
		{ "Exit" },
	};

	BasicMenu start_screen_menu = BasicMenu({ 3, 5 }, start_screen_menu_entries);
	size_t selected = start_screen_menu.get_user_selection();

	std::vector<std::function<std::unique_ptr<Screen>()>> next_screen_mapping = {
		[]() -> std::unique_ptr<Screen> { return std::make_unique<ImportKeychainScreen>(); },
		[]() -> std::unique_ptr<Screen> { return std::make_unique<NewKeychainScreen>(); },
		[]() -> std::unique_ptr<Screen> { return nullptr; },
	};

	return next_screen_mapping[selected]();
}

void show_error(const Point& pos, const std::string& msg) {
	move(pos.row, 0);
	clrtoeol();
	mvaddstr(pos.row, pos.col, msg.c_str());
	addstr(" Press any key to continue.");
	noecho();
	getch();
}

std::unique_ptr<Screen> ImportKeychainScreen::run() {
	return nullptr;
}
