#include <src/cli/manager.h>
#include <src/cli/screens.h>

#include <curses.h>
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
	initscr();
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
	noecho();
	cbreak();
	keypad(stdscr, TRUE);
	nodelay(stdscr, TRUE);

	for (; should_getch;) {
		int ch = getch();

		if (ch == ERR) {
			using namespace std::chrono_literals;
			std::this_thread::sleep_for(10ms);
			continue;
		}

		push_event({EVT::EV_KEY, {ch}});
	}
}

void WindowManager::run() {
	auto getch_thread = std::thread([this]() { this->getch_loop(); });

	std::stack<std::shared_ptr<ScreenController>> controller_stack;

	for (;;) {
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
	}
}
