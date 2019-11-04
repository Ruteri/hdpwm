#pragma once

#include <src/cli/screen_controller.h>
#include <src/cli/fwd.h>

#include <memory>

class NewKeychainScreen : public ScreenController {
	WINDOW *window;
	bool form_posted = false;
	void post_import_form();

	void m_init() override;
	void m_draw() override;
	void m_on_key(int key) override;

  public:
	NewKeychainScreen(WindowManager *wmanager);
};

class ImportKeychainScreen : public ScreenController {
	WINDOW *window;
	bool form_posted = false;
	void post_import_form();

	void m_init() override;
	void m_draw() override;
	void m_on_key(int key) override;

  public:
	ImportKeychainScreen(WindowManager *wmanager);
};
