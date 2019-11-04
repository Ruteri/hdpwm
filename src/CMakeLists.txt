add_subdirectory(utils)
add_subdirectory(crypto)
add_subdirectory(keychain)
add_subdirectory(cli)

add_executable(hdpmanager main.cpp)
target_link_libraries(hdpmanager PRIVATE cli crypto)
