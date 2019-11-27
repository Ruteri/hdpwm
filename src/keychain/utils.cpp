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

#include <src/keychain/utils.h>

namespace keychain {

std::filesystem::path expand_path(const std::string &path) {
	if (path.size() > 0 && path.substr(0, 1) == "~") {
		return std::filesystem::path(getenv("HOME")) / path.substr(2);
	} else {
		return path;
	}
}

Result<void> can_create_db_at_path(const std::filesystem::path &path) {
	if (std::filesystem::exists(path)) {
		return Result<void>::Err("This file already exists, refusing to delete it.");
	} else if (!std::filesystem::is_directory(path.parent_path())) {
		return Result<void>::Err("The parent directory does not exist, create it first.");
	}

	return Result<void>::Ok();
}

Result<void> can_import_db_from_path(const std::filesystem::path &path) {
	if (!std::filesystem::is_directory(path)) {
		return Result<void>::Err("Path seems invalid, refusing to import it.");
	}

	return Result<void>::Ok();
}

Result<UriLocator> parse_uri(const Uri &uri) {
	if (uri.find("file://") == 0) {
		auto path = std::filesystem::path(expand_path(uri.substr(7)));
		return Result<UriLocator>::Ok(path);
	}

	return Result<UriLocator>::Err("Invalid uri");
}

Result<void> can_import_from_uri(const UriLocator &uri) {
	if (auto path = std::get_if<std::filesystem::path>(&uri)) {
		if (!std::filesystem::exists(*path)) {
			return Result<void>::Err("File doesn't exists.");
		} else if (!std::filesystem::is_regular_file(*path)) {
			return Result<void>::Err("Path does not point to a file.");
		}
		return Result<void>::Ok();
	}

	return Result<void>::Err("Invalid uri");
}

Result<void> can_export_to_uri(const UriLocator &uri) {
	if (auto path = std::get_if<std::filesystem::path>(&uri)) {
		if (std::filesystem::exists(*path)) {
			return Result<void>::Err("This file already exists, refusing to delete it.");
		} else if (!std::filesystem::is_directory(path->parent_path())) {
			return Result<void>::Err("The parent directory does not exist, create it first.");
		}
		return Result<void>::Ok();
	}

	return Result<void>::Err("Invalid uri");
}

} // namespace keychain
