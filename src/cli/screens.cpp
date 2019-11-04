#include <src/cli/screens.h>

#include <src/cli/menu.h>

#include <curses.h>

#include <vector>

StartScreen::StartScreen(WindowManager *wmanager) : ScreenController(wmanager) {
	std::vector<BasicMenuEntry> start_screen_menu_entries = {
	    {"Import keychain",
	        [wmanager]() {
		        wmanager->push_controller(std::make_shared<ImportKeychainScreen>(wmanager));
	        }},
	    {"Create new keychain",
	        [wmanager]() {
		        wmanager->push_controller(std::make_shared<NewKeychainScreen>(wmanager));
	        }},
	    {"Exit", [wmanager]() { wmanager->stop(); }},
	};

	start_screen_menu.reset(new BasicMenu(std::move(start_screen_menu_entries), {3, 5}));
}

void StartScreen::m_draw() {
	clear();

	mvaddstr(0, 0, "Deterministic password manager");

	int maxlines = LINES - 1;
	mvaddstr(maxlines, 0, "<return / home> to accept | <up / down arrow> to change | <q> to quit");

	start_screen_menu->draw();
}

void StartScreen::m_on_key(int key) {
	if (key == 'q') {
		wmanager->stop();
	} else {
		start_screen_menu->process_key(key);
	}
}

void ErrorScreen::m_draw() {
	clear();
	mvaddstr(origin.row, origin.col, "Encountered the following error:");
	mvaddstr(origin.row + 1, origin.col + 2, msg.c_str());
	mvaddstr(origin.row + 2, origin.col, "Press any key to continue.");
	// set cursor 0
	noecho();
}

void ErrorScreen::m_on_key(int) { wmanager->pop_controller(); }
