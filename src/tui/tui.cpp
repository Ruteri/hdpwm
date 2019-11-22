/*

Copyright (C) 2019 Mateusz Morusiewicz

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.

*/

#include <src/tui/manager.h>
#include <src/tui/open_keychain_screen.h>
#include <src/tui/new_keychain_screen.h>

#include <src/keychain/utils.h>

#include <external/p-ranav/argparse/include/argparse.hpp>

#include <filesystem>

struct KCConfig {
	std::filesystem::path kc_path;
};

KCConfig process_cmd_line(int argc, const char *argv[]) {
	argparse::ArgumentParser program("hdpwm");

	program.add_argument("-p", "--path")
	  .help("path to keychain data directory")
	  .default_value(std::string{"~/.hdpwm"});

	try {
		program.parse_args(argc, argv);

	} catch (const std::runtime_error &err) {
        if (err.what() == std::string_view{"help called"}) {
			std::cout << program;
			exit(0);
		} else {
			std::cout << err.what() << std::endl;
			std::cout << program;
			exit(1);
		}
	}

	std::string user_provided_path = program.get<std::string>("--path");
	return { keychain::expand_path(user_provided_path) };
}

int main(int argc, const char *argv[]) {

	auto config  = process_cmd_line(argc, argv);

	WindowManager wm;
	if (keychain::can_import_db_from_path(config.kc_path)) {
		// OpenOrExportScreen
		wm.run(std::make_shared<OpenKeychainScreen>(&wm, config.kc_path));
	} else if (keychain::can_create_db_at_path(config.kc_path)) {
		// CreateOrImportScreen
		wm.run(std::make_shared<NewKeychainScreen>(&wm, config.kc_path));
	} else {
		std::cout << "Path " << config.kc_path << " cannot be imported nor is it empty, refusing to continue" << std::endl;
		exit(1);
	}

	return 0;
}
