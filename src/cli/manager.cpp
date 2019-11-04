#include <src/cli/manager.h>
#include <src/cli/screens.h>

#include <curses.h>
#include <signal.h>

#include <thread>
#include <chrono>

namespace {

WindowManager *g_manager;

void resizeHandler(int) {
	if (g_manager) g_manager->on_resize();
}

}

WindowManager::WindowManager() {
	initscr();
	g_manager = this;
	signal(SIGWINCH, resizeHandler);
}

WindowManager::~WindowManager() {
	g_manager = nullptr;
	endwin();
}

void WindowManager::set_controller(ScreenController* new_controller) {
	std::unique_lock<std::mutex> lk(ev_mutex);
	ev_queue.push({ EVT::EV_SET_CONTROLLER, { new_controller } });
	ev_cv.notify_one();
}

void WindowManager::getch_loop() {
	noecho();
	cbreak();
	keypad(stdscr, TRUE);
	nodelay(stdscr, TRUE);

	for (;should_getch;) {
		int ch = getch();

		if (ch == ERR) {
			using namespace std::chrono_literals;
			std::this_thread::sleep_for(10ms);
			continue;
		}

		std::unique_lock<std::mutex> lk(ev_mutex);
		ev_queue.push({ EVT::EV_KEY, { ch } });
		ev_cv.notify_one();
	}
}

void WindowManager::run() {
	auto getch_thread = std::move(std::thread([this](){ this->getch_loop(); }));

	std::unique_ptr<ScreenController> current_controller;

	for (;;) {
		if (current_controller) current_controller->draw();

		WindowEvent ev;

		{
			std::unique_lock<std::mutex> lk(ev_mutex);
            while (ev_queue.empty())
                ev_cv.wait(lk);

            if (ev_queue.empty())
                continue;

            ev = std::move(ev_queue.front());
            ev_queue.pop();
		}

		switch (ev.code) {
		case EVT::EV_KEY:
			if (current_controller) current_controller->on_key(std::get<int>(ev.data));
			break;
		case EVT::EV_RESIZE:
			if (current_controller) current_controller->on_resize();
			break;
		case EVT::EV_SET_CONTROLLER:
			if (current_controller) current_controller->cleanup();
			current_controller.reset(std::move(std::get<ScreenController*>(ev.data)));
			current_controller->init();
			break;
		case EVT::EV_QUIT:
			should_getch = false;
			getch_thread.join();
			if (current_controller) current_controller->cleanup();
			return;
		}
	}
}

void WindowManager::stop() {
	std::unique_lock<std::mutex> lk(ev_mutex);
	ev_queue.push({ EVT::EV_QUIT, {} });
	ev_cv.notify_one();
}

void WindowManager::on_resize() {
	std::unique_lock<std::mutex> lk(ev_mutex);
	ev_queue.push({ EVT::EV_RESIZE, {} });
	ev_cv.notify_one();
}
