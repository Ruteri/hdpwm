#pragma once

#include <src/cli/screen_controller.h>
#include <src/cli/manager.h>
#include <src/cli/utils.h>

class ErrorScreen : public ScreenController {
	Point origin;
	std::string msg;

	void m_draw() override;
	void m_on_key(int) override;

  public:
	ErrorScreen(WindowManager *wmanager, Point origin, std::string msg);
};
