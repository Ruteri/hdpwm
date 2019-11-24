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

#include <src/tui/new_keychain_screen.h>

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

class GenerateKeychainScreen : public ScreenController {
	std::filesystem::path db_path;
	crypto::PasswordHash pw_hash;
	crypto::Seed seed;
	std::vector<std::unique_ptr<StringOutputHandler>> outputs;

	void m_draw() override {
		for (auto &output : outputs) {
			output->draw();
		}
	}

	void m_on_key(int) override {
		auto keychain = keychain::Keychain::initialize_with_seed(
		    this->db_path, std::move(this->seed), std::move(this->pw_hash));
		this->wmanager->set_controller(
		    std::make_shared<KeychainMainScreen>(this->wmanager, std::move(keychain)));
	}

  public:
	GenerateKeychainScreen(
	    WindowManager *wmanager, std::filesystem::path db_path, crypto::PasswordHash pw_hash) :
	    ScreenController(wmanager),
	    db_path(std::move(db_path)), pw_hash(std::move(pw_hash)) {
		std::vector<utils::sensitive_string> mnemonic = crypto::generate_mnemonic(24);

		// TODO(mmorusiewicz): should clear memory after use
		std::string combined_mnemonic;
		for (const auto &word : mnemonic) {
			combined_mnemonic += static_cast<std::string>(word);
			if (word != mnemonic.back()) {
				combined_mnemonic += ' ';
			}
		}

		seed = crypto::mnemonic_to_seed(std::move(mnemonic));

		outputs.push_back(std::make_unique<StringOutputHandler>(Point{3, 5},
		    "Please write down the following mnemonic and press any key to continue."));
		outputs.push_back(std::make_unique<StringOutputHandler>(Point{4, 5}, combined_mnemonic));
	}
};

NewKeychainScreen::NewKeychainScreen(WindowManager *wmanager, const std::filesystem::path &kc_path):
    ScreenController(wmanager), window(stdscr), kc_path(kc_path) {}

void NewKeychainScreen::m_init() {
	if (!form_posted) {
		form_posted = true;
		post_import_form();
	}
}

void NewKeychainScreen::post_import_form() {
	std::shared_ptr<DBInputResult> result = std::make_shared<DBInputResult>();

	auto on_form_done = [this, result]() {
		this->wmanager->pop_controller();

		try {
			this->wmanager->set_controller(std::make_shared<GenerateKeychainScreen>(
			    wmanager, this->kc_path, std::move(result->pw_hash.value())));

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

	form_controller->add_field<SensitiveInputHandler>(
	    on_accept_pw, Point{2, 2}, "Keychain master password: ");

	wmanager->push_controller(form_controller);
}

void NewKeychainScreen::m_draw() {
	wclear(window);

	mvwaddstr(window, 0, 0, "Creating new keychain ");
	waddstr(window, kc_path.c_str());
}

void NewKeychainScreen::m_on_key(int) { wmanager->pop_controller(); }
