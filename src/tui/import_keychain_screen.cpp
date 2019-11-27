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

#include <src/tui/create_keychain_screen.h>
#include <src/tui/error_screen.h>
#include <src/tui/input.h>
#include <src/tui/keychain_main_screen.h>
#include <src/tui/menu.h>

#include <src/crypto/mnemonic.h>
#include <src/crypto/structs.h>
#include <src/crypto/utils.h>
#include <src/keychain/keychain.h>
#include <src/keychain/utils.h>

#include <cassert>
#include <memory>
#include <optional>
#include <string>

ImportKeychainScreen::ImportKeychainScreen(WindowManager *wmanager, std::filesystem::path kc_path) :
    FormController(wmanager) {

	struct FormResult {
		keychain::UriLocator uri;
		crypto::PasswordHash pw_hash;
		crypto::Seed seed;
	};

	std::shared_ptr<FormResult> result = std::make_shared<FormResult>();

	FormController::on_done = [this, result, kc_path]() {
		try {
			auto kc = keychain::Keychain::initialize_with_seed(
			    kc_path, std::move(result->seed), std::move(result->pw_hash));
			kc->import_from_uri(std::move(result->uri));
			this->wmanager->set_controller(
			    std::make_shared<KeychainMainScreen>(this->wmanager, std::move(kc)));
		} catch (const std::exception &e) {
			std::string err_msg = e.what();
			err_msg += ". Please remove ";
			err_msg += kc_path.string();
			err_msg += " directory manually before retrying";
			this->wmanager->set_controller(
			    std::make_shared<ErrorScreen>(this->wmanager, Point{2, 2}, err_msg));
		}
	};
	FormController::on_cancel = [this, kc_path]() {
		this->wmanager->set_controller(
		    std::make_shared<CreateKeychainScreen>(this->wmanager, kc_path));
	};

	std::string title = std::string("Importing keychain ") + kc_path.string();
	FormController::add_label(Point{0, 0}, title);

	auto on_accept_source = [result](const std::string &path) -> bool {
		if (auto locator = keychain::parse_uri(path);
		    locator && keychain::can_import_from_uri(locator.value())) {
			result->uri = locator.value();
			return true;
		}
		return false;
	};

	auto on_accept_pass = [result](const utils::sensitive_string &pw) -> bool {
		result->pw_hash = crypto::hash_password(pw);
		return true;
	};

	auto on_accept_mnemonic = [result](const utils::sensitive_string &mnemonic) -> bool {
		std::vector<utils::sensitive_string> words = crypto::split_mnemonic_words(mnemonic);
		result->seed = crypto::mnemonic_to_seed(words);
		return true;
	};

	FormController::add_field<StringInputHandler>(on_accept_source, Point{2, 2}, "Source (uri): ")
	    ->set_value("file://");

	FormController::add_field<SensitiveInputHandler>(
	    on_accept_pass, Point{4, 2}, "Encryption phrase for new keychain: ")
	    ->set_visible(false);

	FormController::add_field<SensitiveInputHandler>(
	    on_accept_mnemonic, Point{6, 2}, "Mnemonic (space-separated words of the seed phrase): ")
	    ->set_visible(false);
}
