find_package(Curses REQUIRED)
include_directories(${CURSES_INCLUDE_DIR})

add_executable(cli cli.cpp menu.cpp)
target_link_libraries(cli ${CURSES_LIBRARIES})
