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

#include <src/utils/utils.h>

#include <filesystem>
#include <variant>

namespace keychain {

template <typename T = bool> using Result = utils::Result<T>;
using Uri = std::string;

std::filesystem::path expand_path(const std::string &path);

// Or: Result<enum DBPath { can_import, can_create, invalid }>
Result<void> can_import_db_from_path(const std::filesystem::path &path);
Result<void> can_create_db_at_path(const std::filesystem::path &path);

using UriLocator = std::variant<std::filesystem::path>;
Result<UriLocator> parse_uri(const Uri &uri);

Result<void> can_import_from_uri(const UriLocator &uri);
Result<void> can_export_to_uri(const UriLocator &uri);

} // namespace keychain
