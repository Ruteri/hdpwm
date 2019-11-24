#[[

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

]]
set(CURSES_NEED_NCURSES TRUE)

# The packaged ncurses in macosx homebrew are already containing wide-char support
if (NOT ${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(CURSES_NEED_WIDE TRUE)
endif()

find_package(Curses REQUIRED)
include_directories(${CURSES_INCLUDE_DIR})

add_library(tui form_controller.cpp color.cpp output.cpp menu.cpp input.cpp manager.cpp open_keychain_screen.cpp new_keychain_screen.cpp keychain_main_screen.cpp error_screen.cpp help_screen.cpp)
target_link_libraries(tui PUBLIC ${CURSES_LIBRARIES} keychain)
