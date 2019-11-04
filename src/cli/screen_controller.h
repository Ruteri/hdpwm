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

#include <src/cli/manager.h>

class ScreenController {
  private:
	virtual void m_init() {}
	virtual void m_cleanup() {}
	virtual void m_draw() = 0;
	virtual void m_on_resize() {
		this->cleanup();
		this->init();
		this->draw();
	}

	/* TODO: return whether input was processed and should not propagate until top level controller
	 */
	virtual void m_on_key(int key) = 0;

  protected:
	WindowManager *wmanager;

  public:
	ScreenController(WindowManager *wmanager) : wmanager(wmanager) {}
	virtual ~ScreenController() {}

	void init() { this->m_init(); }
	void cleanup() { this->m_cleanup(); }
	void draw() { this->m_draw(); }

	void on_resize() { this->m_on_resize(); }

	void on_key(int key) { this->m_on_key(key); }
};
