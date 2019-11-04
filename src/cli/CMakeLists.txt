find_package(Curses REQUIRED)
include_directories(${CURSES_INCLUDE_DIR})

add_executable(cli form_controller.cpp output.cpp menu.cpp input.cpp manager.cpp screens.cpp new_keychain_screen.cpp keychain_main_screen.cpp cli.cpp)
target_link_libraries(cli ${CURSES_LIBRARIES} crypto keychain stdc++fs)
