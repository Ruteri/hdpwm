add_library(crypto STATIC crypto.cpp structs.cpp mnemonic.cpp timed_encryption_key.cpp mnemonic-wordlist.cpp utils.cpp)
target_link_libraries(crypto PRIVATE cryptopp utils)
