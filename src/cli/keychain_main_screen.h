#pragma once

#include <src/cli/fwd.h>
#include <src/cli/screen_controller.h>

#include <src/keychain/keychain.h>

#include <memory>
#include <vector>

class KeychainMainScreen : public ScreenController {
	enum class State { Browsing, Creating, Editing } state = State::Browsing;

	std::unique_ptr<Keychain> keychain;
	std::shared_ptr<KeychainDirectory> keychain_root_dir;
	std::vector<std::variant<KeychainDirectory *, KeychainEntry *>> flat_entries_cache;
	int c_selected_index = 0;

	int maxlines, maxcols;
	WINDOW *header, *main, *details, *footer;

	void post_entry_form();
	void post_directory_form();

	void post_entry_view(KeychainEntry *entry);
	void post_dir_edit(KeychainDirectory *dir);
	void post_entry_edit(KeychainEntry *entry);

	void draw_entries_box();
	void draw_details_box();

	void m_init() override;
	void m_cleanup() override;
	void m_draw() override;
	void m_on_key(int key) override;

  public:
	KeychainMainScreen(WindowManager *, std::unique_ptr<Keychain>);
	~KeychainMainScreen();
};
