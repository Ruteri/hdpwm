#pragma once

#include <memory>

#include <src/cli/utils.h>

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
public:
	std::unique_ptr<Screen> run() override;
};

void show_error(const Point& pos, const std::string& msg);
