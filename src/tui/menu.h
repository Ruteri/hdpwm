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

#pragma once

#include <src/tui/input.h>
#include <src/tui/utils.h>

#include <functional>
#include <string>
#include <vector>

/* forward declare as ncurses define OK which breaks leveldb */
struct _win_st;
typedef struct _win_st WINDOW;

/* Can also be made template by entry type if needed */
class AnyBasicMenu : public InputHandler {
	Point origin_pos;
	size_t c_selected = 0;

	virtual void m_draw(WINDOW *window) final;

  private:
	/* tokenization interface */
	virtual size_t entries_size() = 0;
	virtual const char *title_at(size_t) = 0;
	virtual void on_accept(size_t) = 0;

  public:
	AnyBasicMenu(Point pos);

	void process_key(int key) final;
};

template <typename EntryType> /* requires std::string title field */
class TokenizedMenu : public AnyBasicMenu {
  public:
	using ValueCallback = std::function<void(const EntryType &)>;
	using UValue = EntryType;

  private:
	std::vector<EntryType> entries;
	ValueCallback on_accept_cb;

	const char *title_at(size_t i) override { return entries[i].title.c_str(); }
	void on_accept(size_t i) override { on_accept_cb(entries[i]); }
	size_t entries_size() override { return entries.size(); }

  public:
	TokenizedMenu(Point pos, std::vector<EntryType> entries, ValueCallback on_accept) :
	    AnyBasicMenu(pos), entries(std::move(entries)), on_accept_cb(std::move(on_accept)) {}
};
