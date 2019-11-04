#pragma once

#include <condition_variable>
#include <memory>
#include <mutex>
#include <variant>
#include <queue>

class ScreenController;

enum class EVT { EV_KEY, EV_RESIZE, EV_SET_CONTROLLER, EV_PUSH_CONTROLLER, EV_POP_CONTROLLER, EV_QUIT };

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
