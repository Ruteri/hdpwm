#include <src/cli/screens.h>

#include <src/cli/menu.h>
#include <src/cli/utils.h>

#include <curses.h>

#include <functional>
#include <memory>
#include <string>
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

bool NewKeychainScreen::process_path(const std::string& path) {
	return true;
}

std::unique_ptr<Screen> NewKeychainScreen::run() {
	cbreak();
	echo();

	clear();

	mvaddstr(0, 0, "Creating new keychain");
	int maxlines = LINES - 1;
	mvaddstr(maxlines, 0, "<shift>-<left arrow> to go back");

	mvaddstr(3, 5, "Database path (max. 256 chars) [~/.hdpwm]: ");
	curs_set(2);

	std::string path;
	for (;;) {

		int ch = getch();
		switch (ch) {
		case KEY_SLEFT: case ERR:
			return std::make_unique<StartScreen>();
		case KEY_ENTER: case KEY_RETURN:
			if (this->process_path(path)) {
				return nullptr;
			} else {
				mvaddstr(4, 5, "Provided path is not valid. Press any key to continue.");
				noecho();
				getch();
				return std::make_unique<StartScreen>();
			}
		}
	}

	return std::make_unique<StartScreen>();
}

std::unique_ptr<Screen> ImportKeychainScreen::run() {
	return nullptr;
}
