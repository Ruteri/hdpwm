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

#pragma once

#include <src/cli/fwd.h>
#include <src/cli/screen_controller.h>

#include <src/keychain/keychain.h>

#include <memory>
#include <vector>

class KeychainMainScreen : public ScreenController {
	enum class State { Browsing, CreatingOrDeleting, Editing } state = State::Browsing;

	std::unique_ptr<keychain::Keychain> m_keychain;
	keychain::Directory::ptr keychain_root_dir;
	std::vector<keychain::AnyKeychainPtr> flat_entries_cache;
	int c_selected_index = 0;

	int maxlines, maxcols;
	WINDOW *header, *main, *details, *footer;

	std::optional<keychain::AnyKeychainPtr> clipboard;
	void paste_into_dir(keychain::Directory::ptr parent_dir);

	void post_entry_form();
	void post_directory_form();

	void post_entry_view(keychain::Entry::ptr entry);
	void post_dir_edit(keychain::Directory::ptr dir);
	void post_entry_edit(keychain::Entry::ptr entry);
	void post_dir_delete(keychain::Directory::ptr dir);
	void post_entry_delete(keychain::Entry::ptr entry);

	void draw_entries_box();
	void draw_details_box();

	void m_init() override;
	void m_cleanup() override;
	void m_draw() override;
	void m_on_key(int key) override;

  public:
	KeychainMainScreen(WindowManager *, std::unique_ptr<keychain::Keychain>);
	~KeychainMainScreen();
};
