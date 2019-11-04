#pragma once

#include <src/cli/utils.h>

#include <src/keychain/keychain.h>

#include <curses.h>

#include <memory>

class Screen {
public:
	virtual std::unique_ptr<Screen> run() = 0;
};

class StartScreen: public Screen {
public:
	std::unique_ptr<Screen> run() override;
};

class NewKeychainScreen: public Screen {
public:
	std::unique_ptr<Screen> run() override;
};

class ImportKeychainScreen: public Screen {
public:
	std::unique_ptr<Screen> run() override;
};

class KeychainMainScreen: public Screen {
	Keychain keychain;
	std::vector<KeychainEntry> keychain_entries;
	int selected_entry = 0;

	int maxlines, maxcols;
	WINDOW *header, *main, *details, *footer;

	void create_windows();
	void delete_windows();
	void draw();

public:
	KeychainMainScreen(Keychain&&);

	std::unique_ptr<Screen> run() override;
};

void show_error(const Point& pos, const std::string& msg);
