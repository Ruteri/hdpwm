#include <src/keychain/keychain_entry.h>

#include <external/nlohmann/json_single_include.h>
using json = nlohmann::json;

namespace keychain {

Entry::ptr deserialize_entry(const json &data, std::weak_ptr<Directory> parent) {
	EntryMeta meta{
	    data["name"].get<std::string>(),
	    data["details"].get<std::string>(),
	    {
	        data["derivation_path"].get<int>(),
	    },
	};

	return std::make_shared<Entry>(meta, parent);
}

Directory::ptr deserialize_directory(const json &data, int dir_level) {
	DirectoryMeta meta{
	    data["name"].get<std::string>(),
	    data["details"].get<std::string>(),
	};

	auto dir = std::make_shared<Directory>(meta, dir_level);

	for (const json &entry : data["entries"]) {
		dir->entries.push_back(deserialize_entry(entry, dir));
	}

	for (const json &child_dir : data["dirs"]) {
		dir->dirs.push_back(deserialize_directory(child_dir, dir_level + 1));
	}

	return dir;
}

json serialize_entry(Entry::ptr entry) {
	return {{"name", entry->meta.name}, {"details", entry->meta.details},
	    {"derivation_path", entry->meta.dpath.seed}};
}

json serialize_directory(Directory::ptr dir) {
	json entries = json::array();
	for (const auto &entry : dir->entries) {
		entries.push_back(serialize_entry(entry));
	}

	json dirs = json::array();
	for (const auto &child_dir : dir->dirs) {
		dirs.push_back(serialize_directory(child_dir));
	}

	return {{"name", dir->meta.name}, {"details", dir->meta.details}, {"dirs", std::move(dirs)},
	    {"entries", std::move(entries)}};
}

namespace {

void process_flatten_dir(std::list<AnyKeychainPtr> *to_visit, Directory::ptr dir) {
	if (!dir->is_open) {
		return;
	}
	for (int i = dir->entries.size() - 1; i >= 0; --i) {
		to_visit->push_front(dir->entries[i]);
	}
	for (int i = dir->dirs.size() - 1; i >= 0; --i) {
		to_visit->push_front(dir->dirs[i]);
	}
}

} // namespace

std::vector<AnyKeychainPtr> flatten_dirs(Directory::ptr root) {
	std::vector<AnyKeychainPtr> rv;
	std::list<AnyKeychainPtr> to_visit{root};

	while (!to_visit.empty()) {
		auto c_node_v = to_visit.front();
		to_visit.pop_front();

		rv.push_back(c_node_v);

		std::visit(overloaded{[](Entry::ptr) {},
		               [&to_visit](Directory::ptr dir) { process_flatten_dir(&to_visit, dir); }},
		    c_node_v);
	}

	return rv;
}

} // namespace keychain
