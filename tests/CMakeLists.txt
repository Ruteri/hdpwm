add_executable(unittests test_main.cpp crypto/test_mnemonic.cpp crypto/test_crypto.cpp crypto/test_utils.cpp keychain/test_keychain.cpp keychain/test_keychain_entry.cpp)
target_link_libraries(unittests PRIVATE crypto keychain)
