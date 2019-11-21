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

#include <src/tui/keychain_main_screen.h>

#include <src/tui/error_screen.h>
#include <src/tui/form_controller.h>
#include <src/tui/help_screen.h>

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
    WindowManager *wmanager, std::unique_ptr<keychain::Keychain> kc) :
    ScreenController(wmanager),
    m_keychain(std::move(kc)) {
	keychain_root_dir = this->m_keychain->get_root_dir();
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

	auto data_string_path = this->m_keychain->get_data_dir_path();
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
	mvwaddstr(this->footer, 0, 2, "<?> for help");

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

auto draw_keychain_directory(WINDOW *win, int row, keychain::Directory::ptr dir) {
	std::string to_print = dir->is_open ? "-" : "+";
	to_print += dir->meta.name;
	mvwaddstr(win, row + 1, 1 + dir->dir_level, to_print.c_str());
}

auto draw_keychain_entry(WINDOW *win, int row, keychain::Entry::ptr entry) {
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
		            keychain::Directory::ptr dir) { draw_keychain_directory(this->main, i, dir); },
		        [this, i](
		            keychain::Entry::ptr entry) { draw_keychain_entry(this->main, i, entry); },
		    },
		    flat_entries_cache[i + n_to_skip]);

		if (state == State::Browsing && i + n_to_skip == this->c_selected_index) {
			wattroff(this->main, A_STANDOUT);
		}
	}
}

void KeychainMainScreen::draw_details_box() {
	wclear(this->details);

	std::visit(
	    overloaded{
	        [](keychain::Directory::ptr) { /* TODO (notes? derivation path?) */ },
	        [this](keychain::Entry::ptr entry) {
		        mvwaddstr(this->details, 1, 0, entry->meta.details.c_str());
	        },
	    },
	    flat_entries_cache[this->c_selected_index]);
}

void KeychainMainScreen::m_draw() {

	if (state != State::CreatingOrDeleting) draw_entries_box();
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
		        [this](keychain::Directory::ptr dir) {
			        dir->is_open ^= 0x1;
			        flat_entries_cache = flatten_dirs(keychain_root_dir);
		        },
		        [this](keychain::Entry::ptr entry) { post_entry_view(entry); },
		    },
		    flat_entries_cache[this->c_selected_index]);
		break;
	case 'n':
		post_entry_form();
		break;
	case 'N':
		post_directory_form();
		break;
	case 'e':
		std::visit(
		    overloaded{
		        [this](keychain::Directory::ptr dir) { post_dir_edit(dir); },
		        [this](keychain::Entry::ptr entry) { post_entry_edit(entry); },
		    },
		    flat_entries_cache[this->c_selected_index]);
		break;
	case 'c':
		clipboard = flat_entries_cache[this->c_selected_index];
		break;
	case 'p':
		if (!clipboard) return;
		std::visit(
		    overloaded{
		        [this](keychain::Directory::ptr dir) { paste_into_dir(dir); },
		        [this](keychain::Entry::ptr entry) { paste_into_dir(entry->parent_dir.lock()); },
		    },
		    flat_entries_cache[this->c_selected_index]);
		break;
	case 'd':
	case 'x':
		clipboard = flat_entries_cache[this->c_selected_index];
		std::visit(
		    overloaded{
		        [this](keychain::Directory::ptr dir) { post_dir_delete(dir); },
		        [this](keychain::Entry::ptr entry) { post_entry_delete(entry); },
		    },
		    flat_entries_cache[this->c_selected_index]);
		break;
	case 'q':
		wmanager->pop_controller();
		break;
	case '?':
		std::vector<const char *> help{"<↑↓> to navigate", "<↲> to view",
		    "<n/N> to add new entry/group", "<e> to edit",
		    "<c|p|x/d> to copy|paste|cut/delete entry or group", "<q> to quit"};
		wmanager->push_controller(std::make_shared<HelpScreen>(wmanager, std::move(help)));
		break;
	}
}

void KeychainMainScreen::paste_into_dir(keychain::Directory::ptr parent_dir) {
	if (!clipboard) return;

	std::visit(
		overloaded{
			[this, parent_dir](keychain::Directory::ptr dir) {
				parent_dir->dirs.push_back(deep_copy_directory(dir, parent_dir));
			},
			[this, parent_dir](keychain::Entry::ptr entry) {
				auto copied_entry = std::make_shared<keychain::Entry>(entry->meta, parent_dir);
				parent_dir->entries.push_back(copied_entry);
			}
		},
		clipboard.value());

	m_keychain->save_entries(keychain_root_dir);
	flat_entries_cache = flatten_dirs(keychain_root_dir);
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
		auto dpath = m_keychain->get_next_derivation_path();
		keychain::EntryMeta new_entry{entry_result->name, entry_result->details, dpath};

		std::visit(
		    overloaded{
		        [this, &new_entry](keychain::Directory::ptr dir) {
			        dir->is_open = true;
			        dir->entries.push_back(std::make_shared<keychain::Entry>(new_entry, dir));
		        },
		        [this, &new_entry](keychain::Entry::ptr entry) {
			        if (auto pd = entry->parent_dir.lock()) {
				        pd->entries.push_back(std::make_shared<keychain::Entry>(new_entry, pd));
			        }
		        },
		    },
		    flat_entries_cache[this->c_selected_index]);

		m_keychain->save_entries(keychain_root_dir);

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

	state = State::CreatingOrDeleting;
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
		keychain::DirectoryMeta new_dir{dir_result->name, ""};

		std::visit(
		    overloaded{
		        [this, &new_dir](keychain::Directory::ptr dir) {
			        dir->is_open = true;
			        dir->dirs.push_back(std::make_shared<keychain::Directory>(new_dir, dir));
		        },
		        [this, &new_dir](keychain::Entry::ptr entry) {
			        if (auto pd = entry->parent_dir.lock()) {
				        pd->dirs.push_back(std::make_shared<keychain::Directory>(new_dir, pd));
			        }
		        },
		    },
		    flat_entries_cache[this->c_selected_index]);

		m_keychain->save_entries(keychain_root_dir);
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

	state = State::CreatingOrDeleting;
	directory_form_controller->add_field<StringInputHandler>("Directory name: ", on_name_accept);

	wmanager->push_controller(std::move(directory_form_controller));
}

void KeychainMainScreen::post_entry_view(keychain::Entry::ptr entry) {
	auto on_form_done = [this]() {
		state = State::Browsing;
		this->wmanager->pop_controller();
	};

	auto entry_view_form =
	    std::make_unique<FormController>(wmanager, this, this->details, on_form_done, on_form_done);

	entry_view_form->add_label(Point{1, 0}, "Name: " + entry->meta.name);

	entry_view_form->add_label(Point{2, 0}, "Secret: ");
	entry_view_form->add_output(std::make_unique<SensitiveOutputHandler>(
	    Point{2, 8}, m_keychain->derive_secret(entry->meta.dpath)));

	entry_view_form->add_label(Point{3, 0}, "Details: " + entry->meta.details);

	state = State::Editing;
	wmanager->push_controller(std::move(entry_view_form));
}

void KeychainMainScreen::post_dir_edit(keychain::Directory::ptr dir) {
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

void KeychainMainScreen::post_entry_edit(keychain::Entry::ptr entry) {
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

void KeychainMainScreen::post_dir_delete(keychain::Directory::ptr dir) {
	auto confirm_result = std::make_shared<std::string>();

	auto on_form_done = [this, confirm_result, dir]() {
		state = State::Browsing;

		if (*confirm_result == "y") {
			auto &parent_dirs = dir->parent_dir.lock()->dirs;
			parent_dirs.erase(std::remove(parent_dirs.begin(), parent_dirs.end(), dir));

			m_keychain->save_entries(keychain_root_dir);
			flat_entries_cache = flatten_dirs(keychain_root_dir);
		}

		this->wmanager->pop_controller();
	};

	auto on_form_cancel = [this]() {
		state = State::Browsing;
		this->wmanager->pop_controller();
	};

	auto confirm_delete_form =
	    std::make_unique<FormController>(wmanager, this, this->main, on_form_done, on_form_cancel);

	auto on_confirm_delete = [confirm_result](const std::string &state) {
		*confirm_result = state;
		return state.empty() || state == "y" || state == "n";
	};

	std::string confirmation_query =
	    "Are you sure you want to delete directory " + dir->meta.name + "? (y/n) [n]: ";
	confirm_delete_form->add_field<StringInputHandler>(
	    Point{1, 0}, confirmation_query, on_confirm_delete);

	state = State::CreatingOrDeleting;
	wmanager->push_controller(std::move(confirm_delete_form));
}

void KeychainMainScreen::post_entry_delete(keychain::Entry::ptr entry) {
	auto confirm_result = std::make_shared<std::string>();

	auto on_form_done = [this, confirm_result, entry]() {
		state = State::Browsing;

		if (*confirm_result == "y") {
			auto &parent_entries = entry->parent_dir.lock()->entries;
			parent_entries.erase(std::remove(parent_entries.begin(), parent_entries.end(), entry));

			m_keychain->save_entries(keychain_root_dir);
			flat_entries_cache = flatten_dirs(keychain_root_dir);
		}

		this->wmanager->pop_controller();
	};

	auto on_form_cancel = [this]() {
		state = State::Browsing;
		this->wmanager->pop_controller();
	};

	auto confirm_delete_form =
	    std::make_unique<FormController>(wmanager, this, this->main, on_form_done, on_form_cancel);

	auto on_confirm_delete = [confirm_result](const std::string &state) {
		*confirm_result = state;
		return state.empty() || state == "y" || state == "n";
	};

	std::string confirmation_query =
	    "Are you sure you want to delete entry " + entry->meta.name + "? (y/n) [n]: ";
	confirm_delete_form->add_field<StringInputHandler>(
	    Point{1, 0}, confirmation_query, on_confirm_delete);

	state = State::CreatingOrDeleting;
	wmanager->push_controller(std::move(confirm_delete_form));
}
