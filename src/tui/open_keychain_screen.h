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

#pragma once

#include <src/tui/fwd.h>
#include <src/tui/screen_controller.h>

#include <filesystem>

class OpenKeychainScreen : public ScreenController {
	WINDOW *window;
	const std::filesystem::path kc_path;

	bool form_posted = false;
	void post_open_form();

	void m_init() override;
	void m_draw() override;
	void m_on_key(int key) override;

  public:
	OpenKeychainScreen(WindowManager *wmanager, const std::filesystem::path &kc_path);
};