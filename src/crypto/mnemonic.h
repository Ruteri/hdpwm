/*

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

*/

#pragma once

#include <src/crypto/structs.h>

#include <string>
#include <vector>

namespace crypto {

struct sensitive_string;

std::vector<utils::sensitive_string> generate_mnemonic(int entropy_size);
Seed mnemonic_to_seed(const std::vector<utils::sensitive_string> &words);

int find_word_index(const utils::sensitive_string &word);

} // namespace crypto
