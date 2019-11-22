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

#include <src/tui/import_keychain_screen.h>

#include <src/tui/error_screen.h>
#include <src/tui/form_controller.h>
#include <src/tui/input.h>
#include <src/tui/keychain_main_screen.h>

#include <src/crypto/crypto.h>
#include <src/crypto/mnemonic.h>
#include <src/crypto/utils.h>
#include <src/keychain/keychain.h>

#include <curses.h>

#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <vector>

struct DBInputResult {
	std::optional<crypto::PasswordHash> pw_hash{};
};

ImportKeychainScreen::ImportKeychainScreen(WindowManager *wmanager, const std::filesystem::path &kc_path) :
    ScreenController(wmanager), window(stdscr), kc_path(kc_path) {}

void ImportKeychainScreen::m_init() {
	if (!form_posted) {
		form_posted = true;
		post_import_form();
	}
}

void ImportKeychainScreen::post_import_form() {
	std::shared_ptr<DBInputResult> result = std::make_shared<DBInputResult>();

	auto on_form_done = [this, result]() {
		this->wmanager->pop_controller();
		try {
			auto keychain =
			    keychain::Keychain::open(this->kc_path, result->pw_hash.value());
			this->wmanager->set_controller(
			    std::make_shared<KeychainMainScreen>(this->wmanager, std::move(keychain)));
		} catch (const std::exception &e) {
			this->wmanager->set_controller(
			    std::make_shared<ErrorScreen>(this->wmanager, Point{2, 5}, e.what()));
		}
	};

	auto on_form_cancel = [this]() {
		this->wmanager->pop_controller();
		this->wmanager->pop_controller();
	};

	auto form_controller =
	    std::make_shared<FormController>(wmanager, this, window, on_form_done, on_form_cancel);

	auto on_accept_pw = [result](const utils::sensitive_string &pw) -> bool {
		result->pw_hash = crypto::hash_password(pw);
		return true;
	};

	form_controller->add_field<SensitiveInputHandler>("Keychain master password: ", on_accept_pw);

	wmanager->push_controller(form_controller);
}

void ImportKeychainScreen::m_draw() {
	wclear(window);

	mvaddstr(0, 0, "Importing keychain ");
	waddstr(window, kc_path.c_str());
}

void ImportKeychainScreen::m_on_key(int) { wmanager->pop_controller(); }
