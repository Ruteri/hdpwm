#include <src/cli/new_keychain_screen.h>

#include <src/cli/error_screen.h>
#include <src/cli/form_controller.h>
#include <src/cli/input.h>
#include <src/cli/keychain_main_screen.h>

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
	std::optional<std::filesystem::path> db_path{};
	std::optional<crypto::PasswordHash> pw_hash{};
};

struct ValidationResult {
	bool valid;
	std::string reason;

	explicit ValidationResult(bool valid = true) : valid(valid), reason("") {}
	explicit ValidationResult(const char *reason) : valid(false), reason(reason) {}

	explicit operator bool() const { return valid; }
};

ValidationResult validate_new_kc_path(const std::filesystem::path &path) {
	if (std::filesystem::exists(path)) {
		return ValidationResult{"This file already exists, refusing to delete it."};
	} else if (!std::filesystem::is_directory(path.parent_path())) {
		return ValidationResult{"The parent directory does not exist, create it first."};
	}

	return ValidationResult{};
}

ValidationResult validate_import_kc_path(const std::filesystem::path &path) {
	if (!std::filesystem::is_directory(path)) {
		return ValidationResult{"Path seems invalid, refusing to import it."};
	}

	return ValidationResult{};
}

std::filesystem::path expand_path(
    const std::string &input_value, const std::string &default_value) {
	std::string path_value = input_value.empty() ? default_value : input_value;

	if (path_value.size() > 0 && path_value.substr(0, 1) == "~") {
		return std::filesystem::path(getenv("HOME")) / path_value.substr(2);
	} else {
		return path_value;
	}
}

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
		std::vector<std::string> mnemonic = crypto::generate_mnemonic(24);

		// TODO(mmorusiewicz): should clear memory after use
		std::string combined_mnemonic;
		for (const std::string &word : mnemonic) {
			combined_mnemonic += word;
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

NewKeychainScreen::NewKeychainScreen(WindowManager *wmanager) :
    ScreenController(wmanager), window(stdscr) {}

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
			    wmanager, std::move(result->db_path.value()), std::move(result->pw_hash.value())));

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
	    std::make_shared<FormController>(wmanager, nullptr, window, on_form_done, on_form_cancel);

	auto on_accept_path = [result](const std::string &path) -> bool {
		result->db_path = expand_path(path, "~/.hdpwm");
		return validate_new_kc_path(result->db_path.value()).valid;
	};

	auto on_accept_pw = [result](const utils::sensitive_string &pw) -> bool {
		result->pw_hash = crypto::hash_password(pw);
		return true;
	};

	form_controller->add_field<StringInputHandler>(
	    "Database path (max. 256 chars) [~/.hdpwm]: ", on_accept_path);
	form_controller->add_field<SensitiveInputHandler>("Keychain master password: ", on_accept_pw);

	wmanager->push_controller(form_controller);
}

void NewKeychainScreen::m_draw() {
	clear();

	mvaddstr(0, 0, "Creating new keychain");
}

void NewKeychainScreen::m_on_key(int) { wmanager->pop_controller(); }

ImportKeychainScreen::ImportKeychainScreen(WindowManager *wmanager) :
    ScreenController(wmanager), window(stdscr) {}

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
			    keychain::Keychain::open(result->db_path.value(), result->pw_hash.value());
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

	auto on_accept_path = [result](const std::string &path) -> bool {
		result->db_path = expand_path(path, "~/.hdpwm");
		return validate_import_kc_path(result->db_path.value()).valid;
	};

	auto on_accept_pw = [result](const utils::sensitive_string &pw) -> bool {
		result->pw_hash = crypto::hash_password(pw);
		return true;
	};

	form_controller->add_field<StringInputHandler>(
	    "Database path (max. 256 chars) [~/.hdpwm]: ", on_accept_path);
	form_controller->add_field<SensitiveInputHandler>("Keychain master password: ", on_accept_pw);

	wmanager->push_controller(form_controller);
}

void ImportKeychainScreen::m_draw() {
	clear();

	mvaddstr(0, 0, "Importing keychain");
}

void ImportKeychainScreen::m_on_key(int) { wmanager->pop_controller(); }
