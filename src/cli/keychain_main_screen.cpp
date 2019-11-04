#include <src/cli/screens.h>

KeychainMainScreen::KeychainMainScreen(Keychain&& keychain): keychain(std::move(keychain)) {}

std::unique_ptr<Screen> KeychainMainScreen::run() {
	return nullptr;
}
