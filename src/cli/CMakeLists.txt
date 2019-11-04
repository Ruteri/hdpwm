set(CURSES_NEED_NCURSES TRUE)
set(CURSES_NEED_WIDE TRUE)

find_package(Curses REQUIRED)

include_directories(${CURSES_INCLUDE_DIR})

add_library(cli form_controller.cpp color.cpp output.cpp menu.cpp input.cpp manager.cpp new_keychain_screen.cpp keychain_main_screen.cpp error_screen.cpp start_screen.cpp)
target_link_libraries(cli PUBLIC ${CURSES_LIBRARIES} keychain)
