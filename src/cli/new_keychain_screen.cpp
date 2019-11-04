#include <src/cli/screens.h>

#include <src/cli/input.h>

#include <src/crypto/mnemonic.h>
#include <src/crypto/utils.h>

#include <curses.h>

#include <filesystem>
#include <memory>
#include <string>
#include <optional>
#include <vector>

// Would also work with std::variant<error, fs::path>
std::optional<std::filesystem::path> get_db_path() {
	StringInputHandler path_input({3, 5}, "Database path (max. 256 chars) [~/.hdpwm]: ");
	input_action_result path_action_result = path_input.process();
	if (path_action_result == input_action_result::BACK) {
		return {};
	}

	std::string path_value = path_input.value.empty() ? "~/.hdpwm" : path_input.value;

	std::filesystem::path path;

	if (path_value.size() > 0 && path_value.substr(0, 1) == "~") {
		path = std::filesystem::path(getenv("HOME")) / path_value.substr(2);
	} else {
		path = path_value;
	}

	return path;
}

bool validate_new_kc_path(const std::filesystem::path &path) {
	if (std::filesystem::exists(path)) {
		show_error({5, 5}, "This file already exists, refusing to delete it.");
		return false;
	} else if (!std::filesystem::is_directory(path.parent_path())) {
		show_error({5, 5}, "The parent directory does not exist, create it first.");
		return false;
	}

	return true;
}

bool validate_import_kc_path(const std::filesystem::path &path) {
	if (!std::filesystem::is_directory(path)) {
		show_error({5, 5}, "Path seems invalid, refusing to import it.");
		return false;
	}

	return true;
}

std::unique_ptr<Screen> NewKeychainScreen::run() {
	clear();

	mvaddstr(0, 0, "Creating new keychain");
	int maxlines = LINES - 1;
	mvaddstr(maxlines, 0, "<shift>-<left arrow> to go back | <return> to continue");

	std::filesystem::path path;
	if (auto provided_path = get_db_path()) {
		path = provided_path.value();
	} else {
		return std::make_unique<StartScreen>();
	}

	if (!validate_new_kc_path(path)) {
		return std::make_unique<StartScreen>();
	}

	SensitiveInputHandler password_input({5, 5}, "Keychain password: ");
	if (password_input.process() == input_action_result::BACK) {
		return std::make_unique<StartScreen>();
	}
	crypto::PasswordHash pw_hash(crypto::hash_password(std::move(password_input.value)));

	std::vector<std::string> mnemonic = crypto::generate_mnemonic(24);
	mvaddstr(7, 5, "Please write down the following mnemonic and press any key to continue.");
	move(8, 5);
	for (const std::string &word : mnemonic) {
		addstr(word.c_str());
		addch(' ');
	}

	getch();

	auto seed = crypto::mnemonic_to_seed(std::move(mnemonic));

	auto keychain = Keychain::initialize_with_seed(path, std::move(seed), std::move(pw_hash));

	return std::make_unique<KeychainMainScreen>(std::move(keychain));
}

std::unique_ptr<Screen> ImportKeychainScreen::run() {
	clear();

	mvaddstr(0, 0, "Opening keychain");
	int maxlines = LINES - 1;
	mvaddstr(maxlines, 0, "<shift>-<left arrow> to go back | <return> to continue");

	std::filesystem::path path;
	if (auto provided_path = get_db_path()) {
		path = provided_path.value();
	} else {
		return std::make_unique<StartScreen>();
	}

	if (!validate_import_kc_path(path)) {
		return std::make_unique<StartScreen>();
	}

	SensitiveInputHandler password_input({5, 5}, "Keychain password: ");
	if (password_input.process() == input_action_result::BACK) {
		return std::make_unique<StartScreen>();
	}
	crypto::PasswordHash pw_hash(crypto::hash_password(std::move(password_input.value)));

	auto keychain = Keychain::open(path, std::move(pw_hash));

	return std::make_unique<KeychainMainScreen>(std::move(keychain));
}
