#include <src/cli/screen_controller.h>

#include <src/cli/menu.h>

class StartScreen : public ScreenController {
	std::unique_ptr<BasicMenu> start_screen_menu;

	void m_draw() override;
	void m_on_key(int key) override;

  public:
	StartScreen(WindowManager *wmanager);
};

