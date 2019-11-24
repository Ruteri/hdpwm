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
#include <src/tui/form_controller.h>
#include <src/tui/input.h>
#include <src/tui/keychain_main_screen.h>
#include <src/tui/menu.h>

#include <src/crypto/crypto.h>
#include <src/crypto/utils.h>
#include <src/keychain/keychain.h>

#include <curses.h>

#include <optional>
#include <memory>
#include <string>

struct FormResult {
	std::optional<crypto::PasswordHash> pw_hash{};
};

OpenKeychainScreen::OpenKeychainScreen(WindowManager *wmanager, const std::filesystem::path &kc_path) :
    ScreenController(wmanager), window(stdscr), kc_path(kc_path) {}

void OpenKeychainScreen::m_init() {
	switch (state) {
	case State::Init:
		if (!this->kc) {
			post_pass_form();
			state = State::Pass;
		} else {
			post_action_form();
			state = State::Action;
		}
		break;
	case State::Pass:
		break;
	case State::AwaitPassCleanup:
		post_action_form();
		state = State::Action;
		break;
	case State::Action:
		break;
	}
}

void OpenKeychainScreen::post_pass_form() {
	using FormResult = std::optional<crypto::PasswordHash>;
	std::shared_ptr<FormResult> result = std::make_shared<FormResult>();

	auto on_form_done = [this, result]() {
		try {
			this->kc = keychain::Keychain::open(this->kc_path, result->value());
			this->state = State::AwaitPassCleanup;
			this->wmanager->pop_controller();
		} catch (const std::exception &e) {
			this->state = State::Init;
			this->wmanager->set_controller(
			    std::make_shared<ErrorScreen>(this->wmanager, Point{2, 5}, e.what()));
		}
	};

	auto on_form_cancel = [this]() {
		this->state = State::Init;
		this->wmanager->pop_controller();
	};

	auto form_controller =
	    std::make_shared<FormController>(wmanager, this, window, on_form_done, on_form_cancel);

	auto on_accept_pw = [result](const utils::sensitive_string &pw) -> bool {
		*result = crypto::hash_password(pw);
		return true;
	};

	form_controller->add_field<SensitiveInputHandler>(
	    on_accept_pw, Point{2, 2}, "Keychain master password: ");

	wmanager->push_controller(form_controller);
}

void OpenKeychainScreen::post_action_form() {
	struct ActionMenuEntry {
		std::string title;
		enum Action { Open, Exit } action;
	};

	std::shared_ptr<ActionMenuEntry::Action> result =
	    std::make_shared<ActionMenuEntry::Action>(ActionMenuEntry::Action::Open);

	auto on_form_done = [this, result]() {
		this->state = State::Init;
		try {
			switch (*result) {
			case ActionMenuEntry::Action::Open:
				this->wmanager->set_controller(/* set over form controller */
				    std::make_shared<KeychainMainScreen>(this->wmanager, this->kc));
				break;
			case ActionMenuEntry::Action::Exit:
				this->wmanager->stop();
				break;
			}
		} catch (const std::exception &e) {
			this->wmanager->set_controller(
			    std::make_shared<ErrorScreen>(this->wmanager, Point{2, 5}, e.what()));
		}
	};

	auto on_form_cancel = [this]() {
		this->state = State::Init;
		this->wmanager->pop_controller();
	};

	auto form_controller =
	    std::make_shared<FormController>(wmanager, this, window, on_form_done, on_form_cancel);

	auto on_accept_menu = [result](const ActionMenuEntry &entry) -> bool {
		*result = entry.action;
		return true;
	};

	std::vector<ActionMenuEntry> menu_entries{
	    {std::string{"Open keychain"}, ActionMenuEntry::Action::Open},
	    {std::string{"Exit"}, ActionMenuEntry::Action::Exit},
	};
	form_controller->add_field<TokenizedMenu<ActionMenuEntry>>(
	    on_accept_menu, Point{2, 2}, std::move(menu_entries));

	wmanager->push_controller(form_controller);
}

void OpenKeychainScreen::m_draw() {
	wclear(window);

	mvaddstr(0, 0, "Opening keychain ");
	waddstr(window, kc_path.c_str());
}

void OpenKeychainScreen::m_on_key(int) { wmanager->pop_controller(); }
