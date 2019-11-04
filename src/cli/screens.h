#pragma once

#include <src/cli/input.h>
#include <src/cli/manager.h>
#include <src/cli/menu.h>
#include <src/cli/output.h>
#include <src/cli/utils.h>

#include <src/keychain/keychain.h>

#include <curses.h>

#include <memory>

class ScreenController {
private:
	virtual void m_init() {}
	virtual void m_cleanup() {}
	virtual void m_draw() = 0;
	virtual void m_on_resize() { this->cleanup(); this->init(); this->draw(); }
	virtual void m_on_key(int key) = 0;

protected:
	WindowManager *wmanager;

public:
	ScreenController(WindowManager *wmanager): wmanager(wmanager) {}

	void init() { this->m_init(); }
	void cleanup() { this->m_cleanup(); }
	void draw() { this->m_draw(); }

	void on_resize() {
		this->m_on_resize();
	}

	void on_key(int key) {
		this->m_on_key(key);
	}
};

class StartScreen: public ScreenController {
	std::unique_ptr<BasicMenu> start_screen_menu;

	void m_draw() override;
	void m_on_key(int key) override;

public:
	StartScreen(WindowManager *wmanager);
};

class ErrorScreen: public ScreenController {
	Point origin;
	std::string msg;
	std::function<void()> on_ok;

	void m_draw() override;
	void m_on_key(int) override;

public:
	ErrorScreen(WindowManager *wmanager, Point origin, std::string msg, std::function<void()> on_ok): ScreenController(wmanager), origin(origin), msg(std::move(msg)), on_ok(on_ok) {}
};

/* Common type for input objects */
/* To be used once views in cli are children-based */
template <typename InputType>
class InputScreenHandler: public ScreenController {
	std::unique_ptr<InputHandler> input;
	using Signal = typename InputType::Signal;

public:
	InputScreenHandler(WindowManager *wmanager, Point origin, std::string title, Signal on_accept, Signal on_cancel): ScreenController(wmanager) {
		input.reset(new InputType(origin, title, on_accept, on_cancel));
	}

	void m_draw() override { input->draw(); }
	void m_on_key(int key) override { input->process_key(key); }
};

class NewKeychainScreen: public ScreenController {
	enum class State { DB_PATH_INPUT, PW_INPUT, MNEMONIC_CONFIRM } state;

	void init_db_path_input();
	void process_db_path_input(std::string &path);
	std::unique_ptr<InputHandler> db_path_input;
	std::optional<std::filesystem::path> db_path;

	void init_password_input();
	void process_password_input(utils::sensitive_string& pw);
	std::unique_ptr<InputHandler> password_input;

	std::unique_ptr<Keychain> keychain;

	std::vector<std::unique_ptr<OutputHandler>> outputs;

	void m_draw() override;
	void m_on_key(int key) override;

public:
	NewKeychainScreen(WindowManager *wmanager);
};

/* class is mostly copy-pase of NewKeychainScreen, but it doesn't
   have to be that so it should not be generalized
*/
class ImportKeychainScreen: public ScreenController {
	enum class State { DB_PATH_INPUT, PW_INPUT } state;

	void init_db_path_input();
	void process_db_path_input(std::string &path);
	std::unique_ptr<InputHandler> db_path_input;
	std::optional<std::filesystem::path> db_path;

	void init_password_input();
	void process_password_input(utils::sensitive_string& pw);
	std::unique_ptr<InputHandler> password_input;

	std::unique_ptr<Keychain> keychain;

	void m_draw() override;
	void m_on_key(int key) override;

public:
	ImportKeychainScreen(WindowManager *wmanager);
};

struct KeychainDirectoryNode;

struct KeychainEntryNode {
	std::string title;
	std::string details;

	KeychainDirectoryNode* parent_dir;
	KeychainEntryNode(const KeychainEntry &entry, KeychainDirectoryNode* parent_dir): title(entry.title), details(entry.details), parent_dir(parent_dir) {}
};

struct KeychainDirectoryNode {
	std::string name;

	KeychainDirectoryNode *parent;
	std::vector<KeychainDirectoryNode> dirs;
	std::vector<KeychainEntryNode> entries;

	int dir_level = 0;
	bool is_open = false;

	KeychainDirectoryNode(const KeychainDirectory &d, KeychainDirectoryNode *parent);
};

class KeychainMainScreen: public ScreenController {
	std::unique_ptr<Keychain> keychain;
	KeychainDirectoryNode *keychain_root_dir;
	std::vector<std::variant<KeychainDirectoryNode*, KeychainEntryNode*>> flat_entries_cache;
	int c_selected_index = 0;

	int maxlines, maxcols;
	WINDOW *header, *main, *details, *footer;

	void m_init() override;
	void m_cleanup() override;
	void m_draw() override;
	void m_on_key(int key) override;

public:
	KeychainMainScreen(WindowManager*, std::unique_ptr<Keychain>);
	~KeychainMainScreen();
};
