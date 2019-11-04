#include <src/cli/menu.h>
#include <src/cli/screens.h>
#include <src/cli/utils.h>

#include <src/crypto/mnemonic.h>

#include <curses.h>

#include <functional>
#include <memory>
#include <string>
#include <vector>

int main() {

	WindowManager wm;
	wm.set_controller(new StartScreen(&wm));
	wm.run();

	return 0;
}
