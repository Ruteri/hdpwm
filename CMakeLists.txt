#[[

Copyright (C) 2019 Mateusz Morusiewicz

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.

]]
cmake_minimum_required (VERSION 3.10)
project (hdpwmanager)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(LEVELDB_BUILD_TESTS OFF)
set(LEVELDB_BUILD_BENCHMARKS OFF)
add_subdirectory(external/leveldb)

set(ARGPARSE_BUILD_TESTS OFF)
add_subdirectory(external/p-ranav/argparse)

add_compile_options(-Wall -Wextra -Wpedantic)

# TODO: add CMakeLists.txt (or at least proper generation) to cryptopp
add_library(cryptopp STATIC IMPORTED)
set_target_properties(cryptopp PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/external/cryptopp/libcryptopp.a)

include_directories(.)

add_subdirectory(src)
add_subdirectory(tests)
