find_package(Curses REQUIRED)
include_directories(${CURSES_INCLUDE_DIR})

add_library(cli form_controller.cpp output.cpp menu.cpp input.cpp manager.cpp screens.cpp new_keychain_screen.cpp keychain_main_screen.cpp)
target_link_libraries(cli PUBLIC ${CURSES_LIBRARIES} leveldb stdc++fs keychain)
