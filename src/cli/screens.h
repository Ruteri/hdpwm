#pragma once

#include <src/cli/input.h>
#include <src/cli/manager.h>
#include <src/cli/menu.h>
#include <src/cli/output.h>
#include <src/cli/utils.h>

#include <src/keychain/keychain.h>

#include <curses.h>

#include <functional>
#include <memory>

class ScreenController {
private:
	virtual void m_init() {}
	virtual void m_cleanup() {}
	virtual void m_draw() = 0;
	virtual void m_on_resize() { this->cleanup(); this->init(); this->draw(); }

	/* TODO: return whether input was processed and should not propagate until top level controller */
	virtual void m_on_key(int key) = 0;

protected:
	WindowManager *wmanager;

public:
	ScreenController(WindowManager *wmanager): wmanager(wmanager) {}
	virtual ~ScreenController() { cleanup(); }

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

	void m_draw() override;
	void m_on_key(int) override;

public:
	ErrorScreen(WindowManager *wmanager, Point origin, std::string msg): ScreenController(wmanager), origin(origin), msg(std::move(msg)) {}
};

class FormController: public ScreenController {
	// non-top level? (or w/ parent)
	ScreenController *parent;
	WINDOW *&window;
	std::function<void()> on_done;

	enum class State { PROCESSING, IGNORING, DONE } state = State::PROCESSING;

	size_t current_input = 0;
	std::vector<InputHandler*> fields = {};

	/* parent has to maintain window and it's pointer */
	int m_cursor_prev_state;
	virtual void m_init() { parent->init(); curs_set(2); }
	virtual void m_cleanup() { parent->cleanup(); curs_set(m_cursor_prev_state); }

	void m_draw() override;
	void m_on_key(int key) override;

	void advance_form();

public:
	FormController(WindowManager *wmanager, ScreenController *parent, WINDOW *&window, std::function<void()> on_done);

	template <typename InputType>
	void add_field(std::string title, std::function<bool(typename InputType::UValue&)> on_accept) {
		fields.push_back(new InputType(Point{2 + (int) fields.size() * 3, 5}, title,
			[this, on_accept](typename InputType::UValue& v) {
				if (on_accept(v)) advance_form();
				else {}
			})
		);
	}
};

class NewKeychainScreen: public ScreenController {
	WINDOW *window;
	std::shared_ptr<FormController> m_form_controller;

	std::optional<std::filesystem::path> db_path;
	std::optional<crypto::PasswordHash> pw_hash;

	void m_init() override;
	void m_draw() override;
	void m_on_key(int key) override;

public:
	NewKeychainScreen(WindowManager *wmanager);
};

class ImportKeychainScreen: public ScreenController {
	WINDOW *window;
	std::shared_ptr<FormController> m_form_controller;

	std::optional<std::filesystem::path> db_path;
	std::optional<crypto::PasswordHash> pw_hash;

	void m_init() override;
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

class NewEntryScreen: public ScreenController {
	std::function<void(KeychainEntry)> on_accept;
	std::function<void()> on_cancel;

	void m_draw() override;
	void m_on_key(int key) override;

public:
	NewEntryScreen(WindowManager *wmanager, decltype(on_accept) on_accept, decltype(on_cancel) on_cancel);
};
