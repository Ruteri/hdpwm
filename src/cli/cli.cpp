#include <string>
#include <vector>
#include <memory>
#include <functional>

#include <curses.h>

#include <src/crypto/mnemonic.h>
#include <src/cli/menu.h>

class Screen {
public:
	virtual std::unique_ptr<Screen> run() = 0;
};

class ImportKeychainScreen: public Screen {
	std::unique_ptr<Screen> run() override {
		return nullptr;
	}
};

class NewKeychainScreen: public Screen {
	std::unique_ptr<Screen> run() override {
		return nullptr;
	}
};

class StartScreen: public Screen {
public:
	std::unique_ptr<Screen> run() {
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
};

int main() {

	initscr();
	cbreak();
	noecho();

	std::unique_ptr<Screen> current_screen = std::make_unique<StartScreen>();
	for (;current_screen;) {
		std::unique_ptr<Screen> next_screen = current_screen->run();
		current_screen = std::move(next_screen);
	}

	endwin();

	return 0;
}
