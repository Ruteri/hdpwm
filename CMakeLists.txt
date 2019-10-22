cmake_minimum_required (VERSION 3.10)
project (hdpwmanager)

add_compile_options(-Wall -Wextra -Wpedantic)

# TODO: add CMakeLists.txt (or at least proper generation) to cryptopp
add_library(cryptopp STATIC IMPORTED)
set_target_properties(cryptopp PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/external/cryptopp/libcryptopp.a)

include_directories(.)

add_subdirectory(src)
add_subdirectory(tests)
