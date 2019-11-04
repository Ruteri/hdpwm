add_library(crypto STATIC mnemonic.cpp mnemonic-wordlist.cpp utils.cpp)
target_link_libraries(crypto PUBLIC cryptopp utils)
