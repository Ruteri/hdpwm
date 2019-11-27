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

#include <src/tui/create_keychain_screen.h>

#include <src/tui/error_screen.h>
#include <src/tui/import_keychain_screen.h>
#include <src/tui/input.h>
#include <src/tui/menu.h>
#include <src/tui/new_keychain_screen.h>

#include <cassert>
#include <memory>
#include <optional>
#include <string>

CreateKeychainScreen::CreateKeychainScreen(
    WindowManager *wmanager, const std::filesystem::path &kc_path) :
    FormController(wmanager) {
	struct ActionMenuEntry {
		std::string title;
		std::function<void()> on_accept;
	};

	this->on_done = [this]() {};
	this->on_cancel = [this]() { this->wmanager->stop(); };

	std::string title = std::string("Creating keychain ") + kc_path.string();
	FormController::add_label(Point{0, 0}, title);

	auto on_accept_menu = [](const ActionMenuEntry &entry) -> bool {
		entry.on_accept();
		return false; // don't propagate
	};

	std::vector<ActionMenuEntry> menu_entries{
	    {std::string{"Create keychain"},
	        [this, kc_path]() {
		        this->wmanager->set_controller(
		            std::make_shared<NewKeychainScreen>(this->wmanager, kc_path));
	        }},
	    {std::string{"Import keychain"},
	        [this, kc_path]() {
		        this->wmanager->set_controller(
		            std::make_shared<ImportKeychainScreen>(this->wmanager, kc_path));
	        }},
	    {std::string{"Exit"}, [this]() { this->wmanager->stop(); }},
	};

	FormController::add_field<TokenizedMenu<ActionMenuEntry>>(
	    on_accept_menu, Point{2, 2}, std::move(menu_entries));
}
