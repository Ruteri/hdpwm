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

#include <src/tui/color.h>
#include <src/tui/error_screen.h>
#include <src/tui/screen_controller.h>
#include <src/tui/utils.h>

#include <curses.h>
#include <locale.h>
#include <signal.h>

#include <chrono>
#include <stack>
#include <thread>

namespace {

WindowManager *g_manager;

void resizeHandler(int) {
	if (g_manager) g_manager->on_resize();
}

} // namespace

WindowManager::WindowManager() {
	setlocale(LC_ALL, "");
	initscr();
	noecho();
	cbreak();
	keypad(stdscr, TRUE);
	nodelay(stdscr, TRUE);
	curs_set(0);
	init_colors();

	g_manager = this;
	signal(SIGWINCH, resizeHandler);
}

WindowManager::~WindowManager() {
	g_manager = nullptr;
	endwin();
}

void WindowManager::push_event(WindowEvent ev) {
	std::unique_lock<std::mutex> lk(ev_mutex);
	ev_queue.push(std::move(ev));
	ev_cv.notify_one();
}

void WindowManager::set_controller(std::shared_ptr<ScreenController> new_controller) {
	push_event({EVT::EV_SET_CONTROLLER, {std::move(new_controller)}});
}

void WindowManager::push_controller(std::shared_ptr<ScreenController> new_controller) {
	push_event({EVT::EV_PUSH_CONTROLLER, {std::move(new_controller)}});
}

void WindowManager::pop_controller() { push_event({EVT::EV_POP_CONTROLLER, {}}); }

void WindowManager::stop() { push_event({EVT::EV_QUIT, {}}); }

void WindowManager::on_resize() { push_event({EVT::EV_RESIZE, {}}); }

void WindowManager::getch_loop() {

	for (; should_getch;) {
		int ch = getch();

		if (ch == ERR) {
			using namespace std::chrono_literals;
			std::this_thread::sleep_for(10ms);
			continue;
		} else if (ch == KEY_RAW_ALT) {
			// will return immediately
			int ch2 = getch();
			if (ch2 == ERR) {
				push_event({EVT::EV_KEY, {KEY_ESC}});
			} else {
				// TODO: add alt-key handling
				push_event({EVT::EV_KEY, {ch2}});
			}
		} else {
			push_event({EVT::EV_KEY, {ch}});
		}
	}
}

void WindowManager::run() {
	auto getch_thread = std::thread([this]() { this->getch_loop(); });

	std::stack<std::shared_ptr<ScreenController>> controller_stack;

	for (;;) {
		try {
			WindowEvent ev;

			std::shared_ptr<ScreenController> current_controller;
			if (!controller_stack.empty()) current_controller = controller_stack.top();

			if (current_controller) current_controller->draw();

			{
				std::unique_lock<std::mutex> lk(ev_mutex);
				while (ev_queue.empty()) ev_cv.wait(lk);

				if (ev_queue.empty()) continue;

				ev = std::move(ev_queue.front());
				ev_queue.pop();
			}

			if (current_controller) {
				switch (ev.code) {
				case EVT::EV_KEY:
					current_controller->on_key(std::get<int>(ev.data));
					break;
				case EVT::EV_RESIZE:
					current_controller->on_resize();
					break;
				case EVT::EV_SET_CONTROLLER:
					current_controller->cleanup();
					controller_stack.pop();
					controller_stack.push(std::get<std::shared_ptr<ScreenController>>(ev.data));
					current_controller = controller_stack.top();
					current_controller->init();
					break;
				case EVT::EV_PUSH_CONTROLLER:
					current_controller->cleanup();
					controller_stack.push(
					    std::move(std::get<std::shared_ptr<ScreenController>>(ev.data)));
					current_controller = controller_stack.top();
					current_controller->init();
					break;
				case EVT::EV_POP_CONTROLLER:
					current_controller->cleanup();
					controller_stack.pop();
					if (!controller_stack.empty()) controller_stack.top()->init();
					break;
				case EVT::EV_QUIT:
					should_getch = false;
					while (!controller_stack.empty()) {
						controller_stack.pop();
					}
					getch_thread.join();
					return;
				}
			} else {
				switch (ev.code) {
				case EVT::EV_SET_CONTROLLER:
				case EVT::EV_PUSH_CONTROLLER:
					controller_stack.push(std::get<std::shared_ptr<ScreenController>>(ev.data));
					controller_stack.top()->init();
					break;
				case EVT::EV_KEY:
				case EVT::EV_RESIZE:
				case EVT::EV_POP_CONTROLLER:
				case EVT::EV_QUIT:
					break;
				}
			}
		} catch (const std::exception &e) {
			if (!controller_stack.empty()) {
				controller_stack.top()->cleanup();
				controller_stack.top()->init();
			}

			controller_stack.push(std::make_shared<ErrorScreen>(this, Point{2, 2}, e.what()));
		}
	}
}
