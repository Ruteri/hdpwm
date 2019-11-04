#pragma once

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
	bool process_path(const std::string& path);

public:
	std::unique_ptr<Screen> run() override;
};

class ImportKeychainScreen: public Screen {
public:
	std::unique_ptr<Screen> run() override;
};

