#include <src/cli/screens.h>

#include <src/cli/menu.h>

#include <curses.h>

#include <vector>

StartScreen::StartScreen(WindowManager *wmanager): ScreenController(wmanager) {
	std::vector<BasicMenuEntry> start_screen_menu_entries = {
		{ "Import keychain", [wmanager]() { wmanager->set_controller(new ImportKeychainScreen(wmanager)); } },
		{ "Create new keychain", [wmanager]() { wmanager->set_controller(new NewKeychainScreen(wmanager)); } },
		{ "Exit", [wmanager]() { wmanager->stop(); } },
	};

	start_screen_menu.reset(new BasicMenu(std::move(start_screen_menu_entries), { 3, 5 }));
}

void StartScreen::m_draw() {
	clear();

	mvaddstr(0, 0, "Deterministic password manager");

	int maxlines = LINES - 1;
	mvaddstr(maxlines, 0, "<right arrow> to accept | <up / down arrow> to change");

	start_screen_menu->draw();
}

void StartScreen::m_on_key(int key) {
	start_screen_menu->process_key(key);
}


void ErrorScreen::m_draw() {
	move(origin.row, 0);
	clear();
	mvaddstr(origin.row, origin.col, msg.c_str());
	addstr(" Press any key to continue.");
	// set cursor 0
	noecho();
}

void ErrorScreen::m_on_key(int) {
	on_ok();
}
