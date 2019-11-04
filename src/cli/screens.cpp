#include <src/cli/screens.h>

#include <src/cli/menu.h>

#include <curses.h>

#include <vector>

StartScreen::StartScreen(WindowManager *wmanager): ScreenController(wmanager) {
	std::vector<BasicMenuEntry> start_screen_menu_entries = {
		{ "Import keychain", [wmanager]() { wmanager->push_controller(std::make_shared<ImportKeychainScreen>(wmanager)); } },
		{ "Create new keychain", [wmanager]() { wmanager->push_controller(std::make_shared<NewKeychainScreen>(wmanager)); } },
		{ "Exit", [wmanager]() { wmanager->stop(); } },
	};

	start_screen_menu.reset(new BasicMenu(std::move(start_screen_menu_entries), { 3, 5 }));
}

void StartScreen::m_draw() {
	clear();

	mvaddstr(0, 0, "Deterministic password manager");

	int maxlines = LINES - 1;
	mvaddstr(maxlines, 0, "<return / home> to accept | <up / down arrow> to change | <q> to quit");

	start_screen_menu->draw();
}

void StartScreen::m_on_key(int key) {
	if (key == 'q') wmanager->stop();
	else start_screen_menu->process_key(key);
}


NewEntryScreen::NewEntryScreen(WindowManager *wmanager, decltype(on_accept) on_accept, decltype(on_cancel) on_cancel): ScreenController(wmanager), on_accept(on_accept), on_cancel(on_cancel) {}

void NewEntryScreen::m_draw() {
	clear();
	refresh();

	// StringInputHandler(const Point& origin, const std::string& title, Signal on_accept, Signal on_cancel);
}

void NewEntryScreen::m_on_key(int key) {
	wmanager->pop_controller();
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
	wmanager->pop_controller();
}
