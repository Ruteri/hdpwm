#include <string>
#include <vector>
#include <memory>
#include <functional>

#include <curses.h>

#include <src/cli/utils.h>
#include <src/crypto/mnemonic.h>
#include <src/cli/menu.h>
#include <src/cli/screens.h>


int main() {

	initscr();

	std::unique_ptr<Screen> current_screen = std::make_unique<StartScreen>();
	for (;current_screen;) {
		std::unique_ptr<Screen> next_screen = current_screen->run();
		current_screen = std::move(next_screen);
	}

	endwin();

	return 0;
}
