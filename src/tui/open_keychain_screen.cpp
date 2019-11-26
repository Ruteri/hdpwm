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

#include <src/tui/open_keychain_screen.h>

#include <src/tui/error_screen.h>
#include <src/tui/export_keychain_screen.h>
#include <src/tui/form_controller.h>
#include <src/tui/input.h>
#include <src/tui/keychain_main_screen.h>
#include <src/tui/menu.h>

#include <src/crypto/crypto.h>
#include <src/crypto/utils.h>
#include <src/keychain/keychain.h>

#include <curses.h>

#include <cassert>
#include <memory>
#include <optional>
#include <string>

OpenKeychainScreen::OpenKeychainScreen(
    WindowManager *wmanager, const std::filesystem::path &kc_path) :
    ScreenController(wmanager),
    window(stdscr), kc_path(kc_path) {}

void OpenKeychainScreen::post_pass_form() {
	using FormResult = std::optional<crypto::PasswordHash>;
	std::shared_ptr<FormResult> result = std::make_shared<FormResult>();

	auto on_form_done = [this, result]() {
		try {
			this->kc = keychain::Keychain::open(this->kc_path, result->value());
			post_action_form();
		} catch (const std::exception &e) {
			this->wmanager->set_controller(
			    std::make_shared<ErrorScreen>(this->wmanager, Point{2, 5}, e.what()));
		}
	};

	auto on_form_cancel = [this]() {
		this->wmanager->pop_controller();
	};

	m_form.reset(new FormController(wmanager, nullptr, window, on_form_done, on_form_cancel));

	std::string title = std::string("Opening keychain ") + kc_path.string();
	m_form->add_label(Point{0, 0}, title);

	auto on_accept_pw = [result](const utils::sensitive_string &pw) -> bool {
		*result = crypto::hash_password(pw);
		return true;
	};

	m_form->add_field<SensitiveInputHandler>(
	    on_accept_pw, Point{2, 2}, "Keychain master password: ");
}

void OpenKeychainScreen::post_action_form() {
	struct ActionMenuEntry {
		std::string title;
		enum Action { Open, Export, Exit } action;
	};

	std::shared_ptr<ActionMenuEntry::Action> result =
	    std::make_shared<ActionMenuEntry::Action>(ActionMenuEntry::Action::Open);

	auto on_form_done = [this, result]() {
		try {
			switch (*result) {
			case ActionMenuEntry::Action::Open:
				this->wmanager->push_controller(
				    std::make_shared<KeychainMainScreen>(this->wmanager, this->kc));
				break;
			case ActionMenuEntry::Action::Export:
				this->wmanager->push_controller(
				    std::make_shared<ExportKeychainScreen>(this->wmanager, this->kc));
				break;
			case ActionMenuEntry::Action::Exit:
				this->wmanager->stop();
				break;
			}
		} catch (const std::exception &e) {
			this->wmanager->push_controller(
			    std::make_shared<ErrorScreen>(this->wmanager, Point{2, 5}, e.what()));
		}

		m_form.reset();
	};

	auto on_form_cancel = [this]() {
		this->wmanager->pop_controller();
	};

	m_form.reset(new FormController(wmanager, nullptr, window, on_form_done, on_form_cancel));

	std::string title = std::string("Opening keychain ") + kc_path.string();
	m_form->add_label(Point{0, 0}, title);

	auto on_accept_menu = [result](const ActionMenuEntry &entry) -> bool {
		*result = entry.action;
		return true;
	};

	std::vector<ActionMenuEntry> menu_entries{
	    {std::string{"Open keychain"}, ActionMenuEntry::Action::Open},
	    {std::string{"Export keychain"}, ActionMenuEntry::Action::Export},
	    {std::string{"Exit"}, ActionMenuEntry::Action::Exit},
	};

	m_form->add_field<TokenizedMenu<ActionMenuEntry>>(
	    on_accept_menu, Point{2, 2}, std::move(menu_entries));
}

void OpenKeychainScreen::m_init() {
	if (!m_form) {
		if (kc /* and master seed decrypted */) {
			post_action_form();
		} else {
			post_pass_form();
		}
	}

	if (m_form) m_form->init();
}

void OpenKeychainScreen::m_cleanup() {
	if (m_form) m_form->cleanup();
}

void OpenKeychainScreen::m_draw() {
	if (m_form) m_form->draw();
}

void OpenKeychainScreen::m_on_key(int k) {
	if (m_form) m_form->on_key(k);
}
