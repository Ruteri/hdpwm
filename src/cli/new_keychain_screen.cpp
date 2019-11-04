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

struct DBInputResult {
	std::optional<std::filesystem::path> db_path{};
	std::optional<crypto::PasswordHash> pw_hash{};
};

struct ValidationResult {
	bool valid;
	std::string reason;

	ValidationResult(bool valid = true): valid(valid), reason("") {}
	ValidationResult(const char *reason): valid(false), reason(reason) {}
	ValidationResult(std::string reason): valid(false), reason(reason) {}

	operator bool() const { return valid; }
};

ValidationResult validate_new_kc_path(std::filesystem::path &path) {
	if (std::filesystem::exists(path)) {
		return { "This file already exists, refusing to delete it." };
	} else if (!std::filesystem::is_directory(path.parent_path())) {
		return { "The parent directory does not exist, create it first." };
	}

	return {};
}

ValidationResult validate_import_kc_path(std::filesystem::path &path) {
	if (!std::filesystem::is_directory(path)) {
		return { "Path seems invalid, refusing to import it." };
	}

	return {};
}

std::filesystem::path expand_path(const std::string& input_value, const std::string& default_value) {
	std::string path_value = input_value.empty() ? default_value : input_value;

	if (path_value.size() > 0 && path_value.substr(0, 1) == "~") {
		return std::filesystem::path(getenv("HOME")) / path_value.substr(2);
	} else {
		return path_value;
	}
}

class GenerateKeychainScreen: public ScreenController {
	std::filesystem::path db_path;
	crypto::PasswordHash pw_hash;
	crypto::Seed seed;
	std::vector<std::unique_ptr<OutputHandler>> outputs;

	void m_draw() override {
		for (auto &output : outputs) {
			output->draw();
		}
	}

	void m_on_key(int) override {
		auto keychain = std::move(Keychain::initialize_with_seed(this->db_path, std::move(this->seed), std::move(this->pw_hash)));
		this->wmanager->set_controller(std::make_shared<KeychainMainScreen>(this->wmanager, std::move(keychain)));
	}

public:
	GenerateKeychainScreen(WindowManager *wmanager, std::filesystem::path db_path, crypto::PasswordHash pw_hash): ScreenController(wmanager), db_path(std::move(db_path)), pw_hash(std::move(pw_hash)) {
		std::vector<std::string> mnemonic = crypto::generate_mnemonic(24);

		// TODO: should clear memory after use
		std::string combined_mnemonic;
		for (const std::string &word : mnemonic) {
			combined_mnemonic += word;
			if (word != mnemonic.back()) {
				combined_mnemonic += ' ';
			}
		}

		seed = std::move(crypto::mnemonic_to_seed(std::move(mnemonic)));

		outputs.push_back(std::make_unique<OutputHandler>(Point{3, 5}, "Please write down the following mnemonic and press any key to continue."));
		outputs.push_back(std::make_unique<OutputHandler>(Point{4, 5}, combined_mnemonic));
	}
};

NewKeychainScreen::NewKeychainScreen(WindowManager *wmanager): ScreenController(wmanager), window(stdscr) {}

void NewKeychainScreen::m_init() {
	if (!m_form_controller) post_import_form();
}

void NewKeychainScreen::post_import_form() {
	std::shared_ptr<DBInputResult> result = std::make_shared<DBInputResult>();

	auto on_form_done = [this, result]() {
		this->wmanager->pop_controller();
		m_form_controller->cleanup();
		try {
			this->wmanager->set_controller(std::make_shared<GenerateKeychainScreen>(wmanager, std::move(result->db_path.value()), std::move(result->pw_hash.value())));

		} catch(const std::exception& e) {
			this->wmanager->set_controller(std::make_shared<ErrorScreen>(this->wmanager, Point{2, 5}, e.what()));
		}
	};

	m_form_controller = std::make_shared<FormController>(wmanager, this, window, on_form_done);

	auto on_accept_path = [result](std::string &path) -> bool {
		result->db_path = expand_path(path, "~/.hdpwm");
		return validate_new_kc_path(result->db_path.value()).valid;
	};

	auto on_accept_pw = [result](utils::sensitive_string& pw) -> bool {
		result->pw_hash = std::move(crypto::hash_password(pw));
		return true;
	};

	m_form_controller->add_field<StringInputHandler>("Database path (max. 256 chars) [~/.hdpwm]: ", on_accept_path);
	m_form_controller->add_field<SensitiveInputHandler>("Keychain master password: ", on_accept_pw);

	wmanager->push_controller(m_form_controller);
}

void NewKeychainScreen::m_draw() {
	clear();

	mvaddstr(0, 0, "Creating new keychain");
	int maxlines = LINES - 1;
	mvaddstr(maxlines, 0, "<shift>-<left arrow> to go back | <return> to continue");
}

void NewKeychainScreen::m_on_key(int) { wmanager->pop_controller(); }


ImportKeychainScreen::ImportKeychainScreen(WindowManager *wmanager): ScreenController(wmanager), window(stdscr) {}

void ImportKeychainScreen::m_init() {
	if (!m_form_controller) post_import_form();
}

void ImportKeychainScreen::post_import_form() {
	std::shared_ptr<DBInputResult> result = std::make_shared<DBInputResult>();

	auto on_form_done = [this, result]() {
		this->wmanager->pop_controller();
		m_form_controller->cleanup();
		try {
			auto keychain = std::move(Keychain::open(result->db_path.value(), result->pw_hash.value()));
			this->wmanager->set_controller(std::make_shared<KeychainMainScreen>(this->wmanager, std::move(keychain)));
		} catch(const std::exception& e) {
			this->wmanager->set_controller(std::make_shared<ErrorScreen>(this->wmanager, Point{2, 5}, e.what()));
		}
	};

	m_form_controller = std::make_shared<FormController>(wmanager, this, window, on_form_done);

	auto on_accept_path = [result](std::string &path) -> bool {
		result->db_path = expand_path(path, "~/.hdpwm");
		return validate_import_kc_path(result->db_path.value()).valid;
	};

	auto on_accept_pw = [result](utils::sensitive_string& pw) -> bool {
		result->pw_hash = std::move(crypto::hash_password(pw));
		return true;
	};

	m_form_controller->add_field<StringInputHandler>("Database path (max. 256 chars) [~/.hdpwm]: ", on_accept_path);
	m_form_controller->add_field<SensitiveInputHandler>("Keychain master password: ", on_accept_pw);

	wmanager->push_controller(m_form_controller);
}

void ImportKeychainScreen::m_draw() {
	clear();

	mvaddstr(0, 0, "Creating new keychain");
	int maxlines = LINES - 1;
	mvaddstr(maxlines, 0, "<shift>-<left arrow> to go back | <return> to continue");
}

void ImportKeychainScreen::m_on_key(int) { wmanager->pop_controller(); }
