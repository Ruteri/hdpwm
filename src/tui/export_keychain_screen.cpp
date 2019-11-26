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

#include <src/tui/export_keychain_screen.h>

#include <src/tui/error_screen.h>
#include <src/tui/form_controller.h>
#include <src/tui/input.h>
#include <src/tui/menu.h>
#include <src/tui/open_keychain_screen.h>

#include <src/keychain/utils.h>

#include <curses.h>

#include <string>

ExportKeychainScreen::ExportKeychainScreen(
    WindowManager *wmanager, std::shared_ptr<keychain::Keychain> kc) :
    ScreenController(wmanager),
    window(stdscr), kc(kc) {
	post_export_form();
}

struct ExportFormResult {
	/* fs path (TODO: or url) */
	keychain::UriLocator uri_locator;
};

void ExportKeychainScreen::post_export_form() {
	std::shared_ptr<ExportFormResult> result = std::make_shared<ExportFormResult>();

	auto on_form_done = [this, result]() {
		try {
			this->kc->export_to_uri(result->uri_locator);
			this->wmanager->pop_controller();
		} catch (const std::exception &e) {
			this->wmanager->set_controller(
			    std::make_shared<ErrorScreen>(this->wmanager, Point{2, 5}, e.what()));
		}
	};

	auto on_form_cancel = [this]() { this->wmanager->pop_controller(); };

	export_fc.reset(new FormController(wmanager, nullptr, window, on_form_done, on_form_cancel));

	auto on_accept_dest = [result](const std::string &path) -> bool {
		auto locator = keychain::parse_uri(path);
		if (locator) {
			result->uri_locator = locator.value();
			return true;
		}
		return false;
	};

	std::string title = std::string("Exporting keychain ") + kc->get_data_dir_path();
	export_fc->add_label(Point{0, 0}, title);

	export_fc->add_field<StringInputHandler>(on_accept_dest, Point{2, 2}, "Destination (uri): ")
	    ->set_value("file://");
}

void ExportKeychainScreen::m_init() { export_fc->init(); }

void ExportKeychainScreen::m_draw() { export_fc->draw(); }

void ExportKeychainScreen::m_on_key(int k) { export_fc->on_key(k); }
