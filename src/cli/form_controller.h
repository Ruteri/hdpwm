#pragma once

#include <src/cli/fwd.h>
#include <src/cli/input.h>
#include <src/cli/manager.h>
#include <src/cli/output.h>
#include <src/cli/screen_controller.h>

#include <functional>
#include <memory>
#include <vector>

class FormController : public ScreenController {
	ScreenController *parent;
	WINDOW *&window;
	std::function<void()> on_done;
	std::function<void()> on_cancel;

	enum class State { PROCESSING, IGNORING, DONE } state = State::PROCESSING;

	std::vector<std::unique_ptr<OutputHandler>> labels = {};

	// TODO: use unique_ptrs
	std::vector<InputHandler *> fields = {};
	size_t current_input = 0;

	/* parent has to maintain window and it's pointer */
	int m_cursor_prev_state;
	void m_init() override;
	void m_cleanup() override;

	void m_draw() override;
	void m_on_key(int key) override;

	void advance_form();

  public:
	FormController(WindowManager *wmanager, ScreenController *parent, WINDOW *&window,
	    std::function<void()> on_done, std::function<void()> on_cancel);

	void add_label(std::string);

	template <typename InputType>
	void add_field(std::string title, std::function<bool(typename InputType::UValue &)> on_accept) {
		Point origin{2 + (int)(fields.size() + labels.size()) * 3, 5};
		fields.push_back(new InputType(
		    std::move(origin), std::move(title), [this, on_accept](typename InputType::UValue &v) {
			    if (on_accept(v))
				    advance_form();
			    else {
			    }
		    }));
	}
};
