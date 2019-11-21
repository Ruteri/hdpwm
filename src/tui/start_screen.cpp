/*

Copyright (C) 2019 Mateusz Morusiewicz

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.

*/

#include <src/tui/start_screen.h>

#include <src/tui/help_screen.h>
#include <src/tui/new_keychain_screen.h>

#include <curses.h>

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
	mvaddstr(maxlines, 2, "<?> for help");

	start_screen_menu->draw();
}

void StartScreen::m_on_key(int key) {
	switch (key) {
	case 'q':
		wmanager->stop();
		break;
	case '?': {
		std::vector<const char *> help{"<↑↓> to navigate", "<↲> to accept", "<q> to quit"};
		wmanager->push_controller(std::make_shared<HelpScreen>(wmanager, std::move(help)));
		break;
	}
	default:
		start_screen_menu->process_key(key);
		break;
	}
}
