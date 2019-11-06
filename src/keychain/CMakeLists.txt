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
include_directories(${LEVELDB_PUBLIC_INCLUDE_DIR})

add_library(keychain STATIC keychain.cpp db.cpp keychain_entry.cpp)

# With OSX 10.15 stdc++fs is build in in libc++, no need to link it separately
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    target_link_libraries(keychain PUBLIC crypto PRIVATE leveldb utils)
else()
    target_link_libraries(keychain PUBLIC stdc++fs crypto PRIVATE leveldb utils)
endif()
