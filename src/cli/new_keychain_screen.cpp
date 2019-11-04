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

struct ValidationResult {
	bool valid;
	std::string reason;

	ValidationResult(bool valid = true): valid(valid), reason("") {}
	ValidationResult(std::string reason): valid(false), reason(reason) {}

	operator bool() const { return valid; }
};

ValidationResult validate_new_kc_path(const std::filesystem::path &path) {
	if (std::filesystem::exists(path)) {
		return { "This file already exists, refusing to delete it." };
	} else if (!std::filesystem::is_directory(path.parent_path())) {
		return { "The parent directory does not exist, create it first." };
	}

	return {};
}

ValidationResult validate_import_kc_path(const std::filesystem::path &path) {
	if (!std::filesystem::is_directory(path)) {
		return { "Path seems invalid, refusing to import it." };
	}

	return { true };
}

std::filesystem::path expand_path(const std::string& input_value, const std::string& default_value) {
	std::string path_value = input_value.empty() ? default_value : input_value;

	if (path_value.size() > 0 && path_value.substr(0, 1) == "~") {
		return std::filesystem::path(getenv("HOME")) / path_value.substr(2);
	} else {
		return path_value;
	}
}

NewKeychainScreen::NewKeychainScreen(WindowManager *wmanager): ScreenController(wmanager) {
	init_db_path_input();
}

void NewKeychainScreen::init_db_path_input() {
	auto title = "Database path (max. 256 chars) [~/.hdpwm]: ";
	Point origin{3, 5};

	auto on_accept = [this](std::string &path) {
		process_db_path_input(path);
	};

	auto on_cancel = [this](std::string&) {
		wmanager->pop_controller();
	};

	db_path_input.reset(new StringInputHandler(origin, title, on_accept, on_cancel));
}

void NewKeychainScreen::process_db_path_input(std::string &path) {
	db_path = expand_path(path, "~/.hdpwm");

	auto validation_result = validate_new_kc_path(db_path.value());
	if (validation_result) {
		this->state = State::PW_INPUT;
		this->init_password_input();
	} else {
		wmanager->set_controller(std::make_shared<ErrorScreen>(wmanager, Point{2, 5}, validation_result.reason));
	}
}


void NewKeychainScreen::init_password_input() {
	auto title = "Keychain master password: ";
	Point origin{5, 5};

	auto on_accept = [this](utils::sensitive_string& pw) {
		process_password_input(pw);
	};

	auto on_cancel = [this](utils::sensitive_string&) {
		wmanager->pop_controller();
	};

	password_input.reset(new SensitiveInputHandler(origin, title, on_accept, on_cancel));
}

void NewKeychainScreen::process_password_input(utils::sensitive_string& pw) {
	crypto::PasswordHash pw_hash(crypto::hash_password(pw));

	this->state = State::MNEMONIC_CONFIRM;

	std::vector<std::string> mnemonic = crypto::generate_mnemonic(24);

	std::string combined_mnemonic;
	for (const std::string &word : mnemonic) {
		combined_mnemonic += word;
		if (word != mnemonic.back()) {
			combined_mnemonic += ' ';
		}
	}

	outputs.push_back(std::make_unique<OutputHandler>(Point{7, 5}, "Please write down the following mnemonic and press any key to continue."));
	outputs.push_back(std::make_unique<OutputHandler>(Point{8, 5}, combined_mnemonic));

	auto seed = crypto::mnemonic_to_seed(std::move(mnemonic));

	try {
		keychain = std::move(Keychain::initialize_with_seed(db_path.value(), std::move(seed), std::move(pw_hash)));
	} catch(const std::exception& e) {
		wmanager->set_controller(std::make_shared<ErrorScreen>(wmanager, Point{2, 5}, e.what()));
	}
}


void NewKeychainScreen::m_draw() {
	clear();

	mvaddstr(0, 0, "Creating new keychain");
	int maxlines = LINES - 1;
	mvaddstr(maxlines, 0, "<shift>-<left arrow> to go back | <return> to continue");

	if (db_path_input) db_path_input->draw();
	if (password_input) password_input->draw();

	for (auto &output : outputs) {
		output->draw();
	}
}

void NewKeychainScreen::m_on_key(int key) {
	switch(state) {
	case State::DB_PATH_INPUT:
		if (db_path_input) db_path_input->process_key(key);
		break;
	case State::PW_INPUT:
		if (password_input) password_input->process_key(key);
		break;
	case State::MNEMONIC_CONFIRM:
		wmanager->set_controller(std::make_shared<KeychainMainScreen>(wmanager, std::move(keychain)));
		break;
	}
}

ImportKeychainScreen::ImportKeychainScreen(WindowManager *wmanager): ScreenController(wmanager) {
	init_db_path_input();
}

void ImportKeychainScreen::init_db_path_input() {
	auto title = "Database path (max. 256 chars) [~/.hdpwm]: ";
	Point origin{3, 5};

	auto on_accept = [this](std::string &path) {
		process_db_path_input(path);
	};

	auto on_cancel = [this](std::string&) {
		wmanager->pop_controller();
	};

	db_path_input.reset(new StringInputHandler(origin, title, on_accept, on_cancel));
}

void ImportKeychainScreen::process_db_path_input(std::string &path) {
	db_path = expand_path(path, "~/.hdpwm");

	auto validation_result = validate_import_kc_path(db_path.value());
	if (validation_result) {
		this->state = State::PW_INPUT;
		this->init_password_input();
	} else {
		wmanager->set_controller(std::make_shared<ErrorScreen>(wmanager, Point{2, 5}, validation_result.reason));
	}
}


void ImportKeychainScreen::init_password_input() {
	auto title = "Keychain master password: ";
	Point origin{5, 5};

	auto on_accept = [this](utils::sensitive_string& pw) {
		process_password_input(pw);
	};

	auto on_cancel = [this](utils::sensitive_string&) {
		wmanager->pop_controller();
	};

	password_input.reset(new SensitiveInputHandler(origin, title, on_accept, on_cancel));
}

void ImportKeychainScreen::process_password_input(utils::sensitive_string& pw) {
	crypto::PasswordHash pw_hash(crypto::hash_password(pw));

	try {
		keychain = std::move(Keychain::open(db_path.value(), std::move(pw_hash)));
		wmanager->set_controller(std::make_shared<KeychainMainScreen>(wmanager, std::move(keychain)));
	} catch(const std::exception& e) {
		wmanager->set_controller(std::make_shared<ErrorScreen>(wmanager, Point{2, 5}, e.what()));
	}
}

void ImportKeychainScreen::m_draw() {
	clear();

	mvaddstr(0, 0, "Creating new keychain");
	int maxlines = LINES - 1;
	mvaddstr(maxlines, 0, "<shift>-<left arrow> to go back | <return> to continue");

	if (db_path_input) db_path_input->draw();
	if (password_input) password_input->draw();
}

void ImportKeychainScreen::m_on_key(int key) {
	switch(state) {
	case State::DB_PATH_INPUT:
		if (db_path_input) db_path_input->process_key(key);
		break;
	case State::PW_INPUT:
		if (password_input) password_input->process_key(key);
		break;
	}
}
