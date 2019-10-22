add_executable(unittests test_main.cpp crypto/test_mnemonic.cpp)
target_link_libraries(unittests PRIVATE crypto cryptopp)
