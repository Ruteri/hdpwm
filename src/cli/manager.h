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

#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>
#include <variant>

class ScreenController;

enum class EVT {
	EV_KEY,
	EV_RESIZE,
	EV_SET_CONTROLLER,
	EV_PUSH_CONTROLLER,
	EV_POP_CONTROLLER,
	EV_QUIT
};

struct WindowEvent {
	EVT code;
	std::variant<int, std::shared_ptr<ScreenController>> data; // TODO: should be unique_ptr
};

class WindowManager {
	std::mutex ev_mutex;
	std::condition_variable ev_cv;
	std::queue<WindowEvent> ev_queue;

	bool should_getch = true;
	void getch_loop();

	void push_event(WindowEvent ev);

  public:
	WindowManager();
	~WindowManager();

	void run();

	void set_controller(std::shared_ptr<ScreenController> new_controller);
	void push_controller(std::shared_ptr<ScreenController> new_controller);
	void pop_controller(); // or do delete_controller(ScreenController*) if needed
	void stop();

	void on_resize();
};
