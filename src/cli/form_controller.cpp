#include <src/cli/form_controller.h>

#include <curses.h>

FormController::FormController(WindowManager *wmanager, ScreenController *parent, WINDOW *&window,
    std::function<void()> on_done) :
    ScreenController(wmanager),
    parent(parent), window(window), on_done(on_done) {
	m_cursor_prev_state = curs_set(2);
}

void FormController::m_init() {
	if (parent) parent->init();
	curs_set(2);
}

void FormController::m_cleanup() {
	curs_set(m_cursor_prev_state);
	if (parent) parent->cleanup();
}

void FormController::m_draw() {
	for (auto &output : labels) {
		output->draw(window);
	}

	int cx = 0, cy = 0;
	for (size_t i = 0; i < fields.size(); ++i) {
		fields[i]->draw(window);
		if (i == current_input) getyx(window, cx, cy);
	}

	if (current_input <= fields.size() && state == State::PROCESSING) {
		wmove(window, cx, cy);
	}

	wrefresh(window);
}

void FormController::m_on_key(int key) {
	switch (state) {
	case State::IGNORING:
	case State::DONE:
		if (parent) parent->on_key(key);
		break;
	case State::PROCESSING:
		if (fields.empty()) return;
		switch (key) {
		case KEY_UP:
			current_input = current_input <= 0 ? 0 : current_input - 1;
			break;
		case KEY_DOWN:
		case '\t':
			fields[current_input]->process_key(KEY_ENTER);
			break;
		default:
			fields[current_input]->process_key(key);
			break;
		}
	}
}

void FormController::advance_form() {
	if (++current_input >= fields.size()) {
		state = State::DONE;
		on_done();
	}
}

void FormController::add_label(std::string text) {
	Point origin{2 + static_cast<int>(fields.size() + labels.size()) * 3, 5};
	labels.push_back(std::make_unique<OutputHandler>(std::move(origin), std::move(text)));
}
