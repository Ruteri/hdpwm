#pragma once

#include <condition_variable>
#include <memory>
#include <mutex>
#include <variant>
#include <queue>

class ScreenController;

enum class EVT { EV_KEY, EV_RESIZE, EV_SET_CONTROLLER, EV_QUIT };

struct WindowEvent {
	EVT code;
	std::variant<int, ScreenController*> data; // TODO: should be unique_ptr
};

class WindowManager {
    std::mutex ev_mutex;
    std::condition_variable ev_cv;
	std::queue<WindowEvent> ev_queue;

	bool should_getch = true;
	void getch_loop();

public:
	WindowManager();
	~WindowManager();

	void run();

	void set_controller(ScreenController* new_controller);
	void stop();

	void on_resize();
};
