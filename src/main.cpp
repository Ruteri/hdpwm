#include <src/cli/manager.h>
#include <src/cli/start_screen.h>

int main() {

	WindowManager wm;
	wm.set_controller(std::make_shared<StartScreen>(&wm));
	wm.run();

	return 0;
}
