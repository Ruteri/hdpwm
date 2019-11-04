#include <src/cli/screens.h>

KeychainMainScreen::KeychainMainScreen(WindowManager *wmanager, std::unique_ptr<Keychain> keychain): ScreenController(wmanager), keychain(std::move(keychain)), keychain_entries(this->keychain->get_entries()) {}

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
	int n_to_skip = std::max(0, this->selected_entry - max_entries + 1);
	for (int i = 0; i < std::min(max_entries, (int) this->keychain_entries.size()); ++i) {
		wmove(this->main, i+1, 0);
		wclrtoeol(this->main);
		if (i + n_to_skip == this->selected_entry) {
			wattron(this->main, A_STANDOUT);
			mvwaddstr(this->main, i+1, 2, this->keychain_entries[i + n_to_skip].title.c_str());
			wattroff(this->main, A_STANDOUT);
		} else {
			mvwaddstr(this->main, i+1, 2, this->keychain_entries[i + n_to_skip].title.c_str());
		}
	}

	/*** details box ***/

	wmove(this->details, 1, 0);
	wclrtoeol(this->details);
	mvwaddstr(this->details, 1, 0, this->keychain_entries[this->selected_entry].details.c_str());

	wrefresh(this->main);
	wrefresh(this->details);
}

void KeychainMainScreen::m_on_key(int key) {

	switch(key) {
	case KEY_DOWN:
		this->selected_entry = (this->selected_entry + 1) % this->keychain_entries.size();
		break;
	case KEY_UP:
		this->selected_entry = (this->keychain_entries.size() + this->selected_entry - 1) % this->keychain_entries.size();
		break;
	case KEY_RIGHT: case KEY_ENTER: case KEY_RETURN:
		break;
	}
}
