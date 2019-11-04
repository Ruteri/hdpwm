add_executable(manager main.cpp)
target_link_libraries(manager PRIVATE crypto cryptopp)

add_subdirectory(crypto)
add_subdirectory(cli)
