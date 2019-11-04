include_directories(${LEVELDB_PUBLIC_INCLUDE_DIR})

add_library(keychain STATIC keychain.cpp)
target_link_libraries(keychain PUBLIC leveldb stdc++fs PRIVATE crypto utils)
