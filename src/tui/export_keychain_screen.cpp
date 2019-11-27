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
    FormController(wmanager) {

	struct ExportFormResult {
		/* fs path (TODO: or url) */
		std::shared_ptr<keychain::Keychain> kc;
		keychain::UriLocator uri_locator;
	};

	std::shared_ptr<ExportFormResult> result = std::make_shared<ExportFormResult>();
	result->kc = std::move(kc);

	FormController::on_done = [this, result]() {
		try {
			result->kc->export_to_uri(result->uri_locator);
			this->wmanager->pop_controller();
		} catch (const std::exception &e) {
			this->wmanager->set_controller(
			    std::make_shared<ErrorScreen>(this->wmanager, Point{2, 5}, e.what()));
		}
	};

	FormController::on_cancel = [this]() { this->wmanager->pop_controller(); };

	auto on_accept_dest = [result](const std::string &path) -> bool {
		if (auto locator = keychain::parse_uri(path);
		    locator && keychain::can_export_to_uri(locator.value())) {
			result->uri_locator = locator.value();
			return true;
		}
		return false;
	};

	std::string title = std::string("Exporting keychain ") + result->kc->get_data_dir_path();
	FormController::add_label(Point{0, 0}, title);

	FormController::add_field<StringInputHandler>(
	    on_accept_dest, Point{2, 2}, "Destination (uri): ")
	    ->set_value("file://");
}
