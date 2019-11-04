find_package(Curses REQUIRED)
include_directories(${CURSES_INCLUDE_DIR})

add_executable(cli cli.cpp screens.cpp new_keychain_screen.cpp keychain_main_screen.cpp menu.cpp input.cpp)
target_link_libraries(cli ${CURSES_LIBRARIES} crypto keychain stdc++fs)
