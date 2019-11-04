#include <src/cli/keychain_main_screen.h>

#include <src/cli/error_screen.h>
#include <src/cli/form_controller.h>

#include <curses.h>

#include <list>

struct EntryFormResult {
	std::string name;
	std::string details;
};

struct DirectoryFormResult {
	std::string name;
};

KeychainMainScreen::KeychainMainScreen(
    WindowManager *wmanager, std::unique_ptr<Keychain> keychain) :
    ScreenController(wmanager),
    keychain(std::move(keychain)) {
	keychain_root_dir = this->keychain->get_root_dir();
	flat_entries_cache = flatten_dirs(keychain_root_dir);
}

KeychainMainScreen::~KeychainMainScreen() {
	cleanup();
	if (keychain_root_dir) keychain_root_dir.reset();
}

// TODO(mmorusiewicz): should be called on screen resize
void KeychainMainScreen::m_init() {
	clear();
	refresh();

	getmaxyx(stdscr, this->maxlines, this->maxcols);

	/* header */

	this->header = newwin(1, this->maxcols, 0, 0);

	auto data_string_path = this->keychain->get_data_dir_path();
	mvwaddstr(this->header, 0, 0, "Browsing ");
	waddstr(this->header, data_string_path.c_str());

	wrefresh(this->header);

	/* main */

	this->main = newwin(this->maxlines - 2, this->maxcols / 5 * 3, 1, 0);

	/* details */

	this->details = newwin(
	    this->maxlines - 2, this->maxcols - (this->maxcols / 5 * 3), 1, this->maxcols / 5 * 3);

	/* footer */

	this->footer = newwin(1, this->maxcols / 5 * 3, this->maxlines - 1, 0);
	mvwaddstr(this->footer, 0, 0,
	    "<↑↓> to navigate | <n/N> to add new entry/group | <↲> to view | <e> to edit | <q> to quit");

	wrefresh(this->footer);
}

void KeychainMainScreen::m_cleanup() {
	for (WINDOW *win : {header, main, details, footer}) {
		wclear(win);
		wrefresh(win);
		delwin(win);
	}
	header = main = details = footer = nullptr;
}

namespace {

auto draw_keychain_directory(WINDOW *win, int row, KeychainDirectory::ptr dir) {
	std::string to_print = dir->is_open ? "-" : "+";
	to_print += dir->meta.name;
	mvwaddstr(win, row + 1, 1 + dir->dir_level, to_print.c_str());
}

auto draw_keychain_entry(WINDOW *win, int row, KeychainEntry::ptr entry) {
	if (auto pd = entry->parent_dir.lock()) {
		mvwaddstr(win, row + 1, 2 + pd->dir_level, entry->meta.name.c_str());
	}
}

} // namespace

void KeychainMainScreen::draw_entries_box() {
	wclear(this->main);

	int max_entries = this->maxlines - 4;
	int n_to_skip = std::max(0, this->c_selected_index - max_entries + 1);

	for (int i = 0; i < std::min(max_entries, static_cast<int>(flat_entries_cache.size())); ++i) {
		if (state == State::Browsing && i + n_to_skip == this->c_selected_index) {
			wattron(this->main, A_STANDOUT);
		}

		std::visit(
		    overloaded{
		        [this, i](
		            KeychainDirectory::ptr dir) { draw_keychain_directory(this->main, i, dir); },
		        [this, i](KeychainEntry::ptr entry) { draw_keychain_entry(this->main, i, entry); },
		    },
		    flat_entries_cache[i]);

		if (state == State::Browsing && i + n_to_skip == this->c_selected_index) {
			wattroff(this->main, A_STANDOUT);
		}
	}
}

void KeychainMainScreen::draw_details_box() {
	wclear(this->details);

	std::visit(
	    overloaded{
	        [](KeychainDirectory::ptr) { /* TODO (notes? derivation path?) */ },
	        [this](KeychainEntry::ptr entry) {
		        mvwaddstr(this->details, 1, 0, entry->meta.details.c_str());
	        },
	    },
	    flat_entries_cache[this->c_selected_index]);
}

void KeychainMainScreen::m_draw() {

	if (state != State::Creating) draw_entries_box();
	if (state == State::Browsing) draw_details_box();

	wrefresh(this->main);
	wrefresh(this->details);
}

void KeychainMainScreen::m_on_key(int key) {
	switch (key) {
	case KEY_DOWN:
		this->c_selected_index = (this->c_selected_index + 1) % flat_entries_cache.size();
		break;
	case KEY_UP:
		this->c_selected_index = this->c_selected_index <= 0 ? flat_entries_cache.size() - 1
		                                                     : this->c_selected_index - 1;
		break;
	case KEY_ENTER:
	case KEY_RETURN:
		std::visit(
		    overloaded{
		        [this](KeychainDirectory::ptr dir) {
			        dir->is_open ^= 0x1;
			        flat_entries_cache = flatten_dirs(keychain_root_dir);
		        },
		        [this](KeychainEntry::ptr entry) { post_entry_view(entry); },
		    },
		    flat_entries_cache[this->c_selected_index]);
		break;
	case 'e':
		std::visit(
		    overloaded{
		        [this](KeychainDirectory::ptr dir) { post_dir_edit(dir); },
		        [this](KeychainEntry::ptr entry) { post_entry_edit(entry); },
		    },
		    flat_entries_cache[this->c_selected_index]);
		break;
	case 'n':
		post_entry_form();
		break;
	case 'N':
		post_directory_form();
		break;
	case 'q':
		wmanager->pop_controller();
		break;
	}
}

void KeychainMainScreen::post_entry_form() {
	std::shared_ptr<EntryFormResult> entry_result = std::make_shared<EntryFormResult>();

	auto on_form_done = [this, entry_result]() {
		this->wmanager->pop_controller();

		if (!entry_result || entry_result->name.empty()) {
			this->wmanager->set_controller(std::make_shared<ErrorScreen>(
			    this->wmanager, Point{2, 5}, "Invalid form returned"));
		}

		// TODO(mmorusiewicz): generate entry using keychain
		auto dpath = keychain->get_next_derivation_path();
		KeychainEntryMeta new_entry{entry_result->name, entry_result->details, dpath};

		std::visit(
		    overloaded{
		        [this, &new_entry](KeychainDirectory::ptr dir) {
			        dir->is_open = true;
			        dir->entries.push_back(std::make_shared<KeychainEntry>(new_entry, dir));
		        },
		        [this, &new_entry](KeychainEntry::ptr entry) {
			        if (auto pd = entry->parent_dir.lock()) {
				        pd->entries.push_back(std::make_shared<KeychainEntry>(new_entry, pd));
			        }
		        },
		    },
		    flat_entries_cache[this->c_selected_index]);

		keychain->save_entries(keychain_root_dir);

		state = State::Browsing;
		flat_entries_cache = flatten_dirs(keychain_root_dir);
	};

	auto on_form_cancel = [this]() {
		state = State::Browsing;
		this->wmanager->pop_controller();
	};

	auto entry_form_controller =
	    std::make_unique<FormController>(wmanager, this, this->main, on_form_done, on_form_cancel);

	auto on_name_accept = [entry_result](const std::string &name) -> bool {
		entry_result->name = name;
		return !name.empty();
	};

	auto on_details_accept = [entry_result](const std::string &details) -> bool {
		entry_result->details = details;
		return true;
	};

	entry_form_controller->add_field<StringInputHandler>("Entry name: ", on_name_accept);
	entry_form_controller->add_field<StringInputHandler>("Details: ", on_details_accept);

	state = State::Creating;
	wmanager->push_controller(std::move(entry_form_controller));
}

void KeychainMainScreen::post_directory_form() {
	std::shared_ptr<DirectoryFormResult> dir_result = std::make_shared<DirectoryFormResult>();

	auto on_form_done = [this, dir_result]() {
		this->wmanager->pop_controller();

		if (!dir_result || dir_result->name.empty()) {
			this->wmanager->set_controller(std::make_shared<ErrorScreen>(
			    this->wmanager, Point{2, 5}, "Invalid form returned"));
		}

		// TODO(mmorusiewicz): generate entry using keychain
		KeychainDirectoryMeta new_dir{dir_result->name, ""};

		std::visit(
		    overloaded{
		        [this, &new_dir](KeychainDirectory::ptr dir) {
			        dir->is_open = true;
			        dir->dirs.push_back(
			            std::make_shared<KeychainDirectory>(new_dir, dir->dir_level + 1));
		        },
		        [this, &new_dir](KeychainEntry::ptr entry) {
			        if (auto pd = entry->parent_dir.lock()) {
				        pd->dirs.push_back(
				            std::make_shared<KeychainDirectory>(new_dir, pd->dir_level + 1));
			        }
		        },
		    },
		    flat_entries_cache[this->c_selected_index]);

		keychain->save_entries(keychain_root_dir);
		state = State::Browsing;
		flat_entries_cache = flatten_dirs(keychain_root_dir);
	};

	auto on_form_cancel = [this]() {
		state = State::Browsing;
		this->wmanager->pop_controller();
	};

	auto directory_form_controller =
	    std::make_unique<FormController>(wmanager, this, this->main, on_form_done, on_form_cancel);

	auto on_name_accept = [dir_result](const std::string &name) -> bool {
		dir_result->name = name;
		return !name.empty();
	};

	state = State::Creating;
	directory_form_controller->add_field<StringInputHandler>("Directory name: ", on_name_accept);

	wmanager->push_controller(std::move(directory_form_controller));
}

void KeychainMainScreen::post_entry_view(KeychainEntry::ptr entry) {
	auto on_form_done = [this]() {
		state = State::Browsing;
		this->wmanager->pop_controller();
	};

	auto entry_view_form =
	    std::make_unique<FormController>(wmanager, this, this->details, on_form_done, on_form_done);

	entry_view_form->add_label(Point{1, 0}, "Name: " + entry->meta.name);

	entry_view_form->add_label(Point{2, 0}, "Secret: ");
	entry_view_form->add_output(std::make_unique<SensitiveOutputHandler>(
	    Point{2, 8}, keychain->derive_secret(entry->meta.dpath)));

	entry_view_form->add_label(Point{3, 0}, "Details: " + entry->meta.details);

	state = State::Editing;
	wmanager->push_controller(std::move(entry_view_form));
}

void KeychainMainScreen::post_dir_edit(KeychainDirectory::ptr dir) {
	auto on_form_done = [this]() {
		state = State::Browsing;
		this->wmanager->pop_controller();
	};

	auto dir_edit_form =
	    std::make_unique<FormController>(wmanager, this, this->details, on_form_done, on_form_done);

	auto on_name_change_accept = [dir](const std::string &new_name) {
		if (new_name.empty()) return false;
		dir->meta.name = new_name;
		return true;
	};

	auto name_input =
	    dir_edit_form->add_field<StringInputHandler>(Point{1, 0}, "Name: ", on_name_change_accept);
	name_input->set_value(dir->meta.name);

	state = State::Editing;
	wmanager->push_controller(std::move(dir_edit_form));
}

void KeychainMainScreen::post_entry_edit(KeychainEntry::ptr entry) {
	auto on_form_done = [this]() {
		state = State::Browsing;
		this->wmanager->pop_controller();
	};

	auto entry_edit_form =
	    std::make_unique<FormController>(wmanager, this, this->details, on_form_done, on_form_done);

	auto on_name_change_accept = [entry](const std::string &new_name) {
		if (new_name.empty()) return false;
		entry->meta.name = new_name;
		return true;
	};

	auto on_details_change_accept = [entry](const std::string &new_details) {
		if (new_details.empty()) return false;
		entry->meta.details = new_details;
		return true;
	};

	auto name_input = entry_edit_form->add_field<StringInputHandler>(
	    Point{1, 0}, "Name: ", on_name_change_accept);
	name_input->set_value(entry->meta.name);

	entry_edit_form->add_label(Point{2, 0}, "Secret: ");
	entry_edit_form->add_output(std::make_unique<SensitiveOutputHandler>(
	    Point{2, 8}, utils::sensitive_string("somesecret")));

	auto details_input = entry_edit_form->add_field<StringInputHandler>(
	    Point{3, 0}, "Details: ", on_details_change_accept);
	details_input->set_value(entry->meta.details);

	state = State::Editing;
	wmanager->push_controller(std::move(entry_edit_form));
}
