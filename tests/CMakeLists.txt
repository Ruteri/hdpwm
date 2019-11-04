add_executable(unittests test_main.cpp crypto/test_mnemonic.cpp crypto/test_crypto.cpp crypto/test_utils.cpp)
target_link_libraries(unittests PRIVATE crypto)
