#include <src/cli/screens.h>

#include <list>

struct EntryFormResult {
	std::string name;
	std::string details;
};

struct DirectoryFormResult {
	std::string name;
};

// helper for visitors
template <class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template <class... Ts> overloaded(Ts...)->overloaded<Ts...>;

KeychainDirectoryNode::KeychainDirectoryNode(
    const KeychainDirectory &d, KeychainDirectoryNode *parent) {
	this->name = d.name;
	this->parent = parent;
	this->dir_level = parent ? parent->dir_level + 1 : 0;
	for (auto &entry : d.entries) {
		this->entries.emplace_back(entry, this);
	}

	// For now, recurse (it's not deep and done only on startup)
	for (auto &dir : d.dirs) {
		this->dirs.emplace_back(dir, this);
	}
}

std::vector<std::variant<KeychainDirectoryNode *, KeychainEntryNode *>> flatten_dirs(
    KeychainDirectoryNode *root) {
	std::vector<std::variant<KeychainDirectoryNode *, KeychainEntryNode *>> rv;
	std::list<std::variant<KeychainDirectoryNode *, KeychainEntryNode *>> to_visit = {root};

	while (!to_visit.empty()) {
		std::variant<KeychainDirectoryNode *, KeychainEntryNode *> c_node_v = to_visit.front();
		to_visit.pop_front();

		rv.push_back(c_node_v);

		std::visit(overloaded{[&rv](KeychainEntryNode *) {},
		               [&rv, &to_visit](KeychainDirectoryNode *dir) {
			               if (!dir->is_open) {
				               return;
			               }

			               for (int i = dir->entries.size() - 1; i >= 0; --i) {
				               to_visit.push_front(&(dir->entries[i]));
			               }
			               for (int i = dir->dirs.size() - 1; i >= 0; --i) {
				               to_visit.push_front(&(dir->dirs[i]));
			               }
		               }},
		    c_node_v);
	}

	return rv;
}

KeychainMainScreen::KeychainMainScreen(
    WindowManager *wmanager, std::unique_ptr<Keychain> keychain) :
    ScreenController(wmanager),
    keychain(std::move(keychain)) {
	KeychainDirectory root_dir = this->keychain->get_root_dir();
	keychain_root_dir = new KeychainDirectoryNode(root_dir, nullptr);
	keychain_root_dir->is_open = true;

	flat_entries_cache = flatten_dirs(keychain_root_dir);
}
KeychainMainScreen::~KeychainMainScreen() {
	cleanup();
	if (keychain_root_dir) delete keychain_root_dir;
}

// TODO: should be called on screen resize
void KeychainMainScreen::m_init() {
	clear();
	refresh();

	getmaxyx(stdscr, this->maxlines, this->maxcols);

	/* header */

	this->header = newwin(1, this->maxcols / 5 * 4, 0, 0);

	auto data_string_path = this->keychain->get_data_dir_path();
	mvwaddstr(this->header, 0, 0, "Browsing ");
	waddstr(this->header, data_string_path.c_str());

	wrefresh(this->header);

	/* main */

	this->main = newwin(this->maxlines - 2, this->maxcols / 5 * 4, 1, 0);

	/* details */

	this->details = newwin(
	    this->maxlines - 2, this->maxcols - (this->maxcols / 5 * 4), 1, this->maxcols / 5 * 4);

	/* footer */

	this->footer = newwin(1, this->maxcols / 5 * 4, this->maxlines - 1, 0);
	mvwaddstr(this->footer, 0, 0,
	    "<up & down arrows> to navigate | <n> to add new entry | <N> to add new group | <return> to view & edit | <q> to quit");

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

void KeychainMainScreen::m_draw() {

	/*** entries box ***/

	int max_entries = this->maxlines - 4;
	int n_to_skip = std::max(0, this->c_selected_index - max_entries + 1);

	for (int i = 0; i < std::min(max_entries, static_cast<int>(flat_entries_cache.size())); ++i) {
		wmove(this->main, i + 1, 0);
		wclrtoeol(this->main);
		if (i + n_to_skip == this->c_selected_index) {
			wattron(this->main, A_STANDOUT);
		}

		std::visit(
		    overloaded{
		        [this, i](KeychainDirectoryNode *dir) {
			        std::string to_print = dir->is_open ? "-" : "+";
			        to_print += dir->name;
			        mvwaddstr(this->main, i + 1, 1 + dir->dir_level, to_print.c_str());
		        },
		        [this, i](KeychainEntryNode *entry) {
			        std::string to_print = entry->title;
			        mvwaddstr(
			            this->main, i + 1, 2 + entry->parent_dir->dir_level, to_print.c_str());
		        },
		    },
		    flat_entries_cache[i]);

		if (i + n_to_skip == this->c_selected_index) {
			wattroff(this->main, A_STANDOUT);
		}
	}

	wclrtobot(this->main);

	/*** details box ***/

	{
		wmove(this->details, 1, 0);
		wclrtoeol(this->details);

		std::string to_print;
		std::visit(
		    overloaded{
		        [&to_print](KeychainDirectoryNode *) {},
		        [&to_print](KeychainEntryNode *entry) { to_print = entry->details; },
		    },
		    flat_entries_cache[this->c_selected_index]);

		mvwaddstr(this->details, 1, 0, to_print.c_str());
	}

	wclrtobot(this->details);

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
		        [this](KeychainDirectoryNode *dir) {
			        dir->is_open ^= 0x1;
			        flat_entries_cache = flatten_dirs(keychain_root_dir);
		        },
		        [this](KeychainEntryNode *) {
			        // TODO: move to edit screen (or sth like that)
		        },
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
		m_entry_form_controller.reset();

		if (!entry_result || entry_result->name.empty()) {
			this->wmanager->set_controller(std::make_shared<ErrorScreen>(
			    this->wmanager, Point{2, 5}, "Invalid form returned"));
		}

		// TODO: generate entry using keychain
		KeychainEntry new_entry{entry_result->name, entry_result->details};

		std::visit(
		    overloaded{
		        [this, &new_entry](KeychainDirectoryNode *dir) {
			        dir->is_open = true;
			        dir->entries.push_back({new_entry, dir});
		        },
		        [this, &new_entry](KeychainEntryNode *entry) {
			        entry->parent_dir->entries.push_back({new_entry, entry->parent_dir});
		        },
		    },
		    flat_entries_cache[this->c_selected_index]);
		flat_entries_cache = flatten_dirs(keychain_root_dir);
	};

	m_entry_form_controller =
	    std::make_shared<FormController>(wmanager, this, this->main, on_form_done);

	auto on_name_accept = [entry_result](std::string &name) -> bool {
		entry_result->name = name;
		return !name.empty();
	};

	auto on_details_accept = [entry_result](std::string &details) -> bool {
		entry_result->details = details;
		return true;
	};

	m_entry_form_controller->add_field<StringInputHandler>("Entry name: ", on_name_accept);
	m_entry_form_controller->add_field<StringInputHandler>("Details: ", on_details_accept);

	wmanager->push_controller(m_entry_form_controller);
}

void KeychainMainScreen::post_directory_form() {
	std::shared_ptr<DirectoryFormResult> dir_result = std::make_shared<DirectoryFormResult>();

	auto on_form_done = [this, dir_result]() {
		this->wmanager->pop_controller();
		m_directory_form_controller.reset();

		if (!dir_result || dir_result->name.empty()) {
			this->wmanager->set_controller(std::make_shared<ErrorScreen>(
			    this->wmanager, Point{2, 5}, "Invalid form returned"));
		}

		// TODO: generate entry using keychain
		KeychainDirectory new_dir{dir_result->name, {}, {}};

		std::visit(
		    overloaded{
		        [this, &new_dir](KeychainDirectoryNode *dir) {
			        dir->is_open = true;
			        dir->dirs.push_back({new_dir, dir});
		        },
		        [this, &new_dir](KeychainEntryNode *entry) {
			        entry->parent_dir->dirs.push_back({new_dir, entry->parent_dir});
		        },
		    },
		    flat_entries_cache[this->c_selected_index]);
		flat_entries_cache = flatten_dirs(keychain_root_dir);
	};

	m_directory_form_controller =
	    std::make_shared<FormController>(wmanager, this, this->main, on_form_done);

	auto on_name_accept = [dir_result](std::string &name) -> bool {
		dir_result->name = name;
		return !name.empty();
	};

	m_directory_form_controller->add_field<StringInputHandler>("Directory name: ", on_name_accept);

	wmanager->push_controller(m_directory_form_controller);
}
