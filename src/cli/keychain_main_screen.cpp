#include <src/cli/screens.h>

#include <list>

// helper for visitors
template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

KeychainDirectoryNode::KeychainDirectoryNode(const KeychainDirectory &d, KeychainDirectoryNode *parent) {
	this->name = d.name;
	this->parent = parent;
	this->dir_level = parent ? parent->dir_level + 1 : 0;
	for (auto &entry: d.entries) {
		this->entries.emplace_back(entry, this);
	}

	// For now, recurse
	for (auto &dir : d.dirs) {
		this->dirs.emplace_back(dir, this);
	}
}

std::vector<std::variant<KeychainDirectoryNode*, KeychainEntryNode*>> flatten_dirs(KeychainDirectoryNode *root) {
	std::vector<std::variant<KeychainDirectoryNode*, KeychainEntryNode*>> rv;
	std::list<std::variant<KeychainDirectoryNode*, KeychainEntryNode*>> to_visit = {root};

	while (!to_visit.empty()) {
		std::variant<KeychainDirectoryNode*, KeychainEntryNode*> c_node_v = to_visit.front();
		to_visit.pop_front();

		rv.push_back(c_node_v);

		std::visit(overloaded {
		[&rv](KeychainEntryNode*) {},
		[&rv, &to_visit](KeychainDirectoryNode* dir) {
			if (!dir->is_open) {
				return;
			}

			for (int i = dir->entries.size() - 1; i >= 0; --i) {
				to_visit.push_front(&(dir->entries[i]));
			}
			for (int i = dir->dirs.size() - 1; i >= 0; --i) {
				to_visit.push_front(&(dir->dirs[i]));
			}
		}}, c_node_v);
	}

	return rv;
}

KeychainMainScreen::KeychainMainScreen(WindowManager *wmanager, std::unique_ptr<Keychain> keychain): ScreenController(wmanager), keychain(std::move(keychain)) {
	KeychainDirectory root_dir = this->keychain->get_root_dir();
	keychain_root_dir = new KeychainDirectoryNode(root_dir, nullptr);
	keychain_root_dir->is_open = true;

	flat_entries_cache = std::move(flatten_dirs(keychain_root_dir));
}
KeychainMainScreen::~KeychainMainScreen() {
	if (keychain_root_dir) delete keychain_root_dir;
}

// TODO: should be called on screen resize
void KeychainMainScreen::m_init() {
	clear();
	refresh();

	cbreak();
	noecho();
	keypad(stdscr, TRUE);

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

	this->details = newwin(this->maxlines - 2, this->maxcols - (this->maxcols / 5 * 4), 1, this->maxcols / 5 * 4);


	/* footer */

	this->footer = newwin(1, this->maxcols / 5 * 4, this->maxlines - 1, 0);
	mvwaddstr(this->footer, 0, 0, "<up & down arrows> to navigate | <n> to add new entry | <return> to view & edit | <q> to quit");

	wrefresh(this->footer);
}

void KeychainMainScreen::m_cleanup() {
	delwin(this->header);
	delwin(this->main);
	delwin(this->details);
	delwin(this->footer);
}

void KeychainMainScreen::m_draw() {

	/*** entries box ***/

	int max_entries = this->maxlines - 4;
	int n_to_skip = std::max(0, this->c_selected_index - max_entries + 1);

	for (int i = 0; i < std::min(max_entries, (int) flat_entries_cache.size()); ++i) {
		wmove(this->main, i+1, 0);
		wclrtoeol(this->main);
		if (i + n_to_skip == this->c_selected_index) {
			wattron(this->main, A_STANDOUT);
		}

		std::visit(overloaded {
			[this,i](KeychainDirectoryNode* dir) {
				std::string to_print = dir->is_open ? "-" : "+";
				to_print += dir->name;
				mvwaddstr(this->main, i+1, 1 + dir->dir_level, to_print.c_str());
			},
			[this,i](KeychainEntryNode* entry) {
				std::string to_print = entry->title;
				mvwaddstr(this->main, i+1, 2 + entry->parent_dir->dir_level, to_print.c_str());
			},
		}, flat_entries_cache[i]);

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
		std::visit(overloaded {
		[&to_print](KeychainDirectoryNode*) {},
		[&to_print](KeychainEntryNode* entry) { to_print = entry->details; },
		}, flat_entries_cache[this->c_selected_index]);

		mvwaddstr(this->details, 1, 0, to_print.c_str());
	}

	wclrtobot(this->details);

	wrefresh(this->main);
	wrefresh(this->details);
}

void KeychainMainScreen::m_on_key(int key) {
	switch(key) {
	case KEY_DOWN:
		this->c_selected_index = (this->c_selected_index + 1 ) % flat_entries_cache.size();
		break;
	case KEY_UP:
		this->c_selected_index = this->c_selected_index <= 0 ? flat_entries_cache.size() - 1 : this->c_selected_index - 1;
		break;
	case KEY_ENTER: case KEY_RETURN:
		std::visit(overloaded {
		[this](KeychainDirectoryNode* dir) {
			dir->is_open ^= 0x1;
			flat_entries_cache = std::move(flatten_dirs(keychain_root_dir));
		},
		[this](KeychainEntryNode*) {
			// TODO: move to edit screen (or sth like that)
		},
		}, flat_entries_cache[this->c_selected_index]);
		break;
	}
}
