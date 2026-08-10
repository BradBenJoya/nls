#include <filesystem>
#include <fstream>
#include <print>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_set>
#include <algorithm>

const std::unordered_set<std::string_view> knownFlags = {"-a", "-l", "-r", "-t", "-S", "-R", "-d"};

[[nodiscard]] static bool isHidden(const std::filesystem::path& path) {
    // Only really works on POSIX for now
    const std::string filename = path.filename().string();
    return !filename.empty() && filename.at(0) == '.';
}

[[nodiscard]] static char getFileType(const std::filesystem::file_status& status) {
    using enum std::filesystem::file_type;
    switch (status.type()) {
        case directory:return 'd';
        case symlink: return 'l';
        case block: return 'b';
        case character: return 'c';
        case fifo: return 'p';
        case socket: return 's';
        case regular: return '-';
        default: return '?';
    }
}

[[nodiscard]] static std::string getFilePerms(const std::filesystem::path& path, std::filesystem::file_status status) {
    std::string buffer = "----------";
    using std::filesystem::perms;
    buffer.at(0) = getFileType(status);
    // perms is a bitmask; & with a single flag checks whether that flag is set
    if ((status.permissions() & perms::owner_read) != perms::none) {
        buffer.at(1) = 'r';
    }
    if ((status.permissions() & perms::owner_write) != perms::none) {
        buffer.at(2) = 'w';
    }
    if ((status.permissions() & perms::owner_exec) != perms::none) {
        buffer.at(3) = 'x';
    }
    if ((status.permissions() & perms::group_read) != perms::none) {
        buffer.at(4) = 'r';
    }
    if ((status.permissions() & perms::group_write) != perms::none) {
        buffer.at(5) = 'w';
    }
    if ((status.permissions() & perms::group_exec) != perms::none) {
        buffer.at(6) = 'x';
    }
    if ((status.permissions() & perms::others_read) != perms::none) {
        buffer.at(7) = 'r';
    }
    if ((status.permissions() & perms::others_write) != perms::none) {
        buffer.at(8) = 'w';
    }
    if ((status.permissions() & perms::others_exec) != perms::none) {
        buffer.at(9) = 'x';
    }

    buffer += " ";
    buffer += std::to_string(std::filesystem::hard_link_count(path));
    buffer += " ";
    // file_size() throws on directories, so guard it and report 0 instead of crashing
    buffer += std::to_string(
        status.type() == std::filesystem::file_type::directory
            ? 0uz
            : std::filesystem::file_size(path)
    );
    buffer += " ";
    // %b %d %H:%M -> abbreviated month, day, 24h time (e.g. "Aug 09 22:18")
    buffer += std::format("{:%b %d %H:%M}",
        std::chrono::floor<std::chrono::minutes>(
            std::chrono::clock_cast<std::chrono::system_clock>(
                std::filesystem::last_write_time(path)
            )
        )
    );

    return buffer;
}

static std::string loadFileToString(const std::filesystem::path& path) {
    std::ifstream file{path.string()};
    if (!file.is_open()) {
        std::println(std::cerr, "Could not open file: {}", path.string());
        return {};
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

enum class SortMode { None, Time, Size };

int main(int argc, char** argv) {
    std::unordered_set<std::string_view> args(argv, argv + argc);
    std::unordered_set<std::filesystem::path> paths;
    
    SortMode sortMode = SortMode::None;

    for (size_t i{1uz}; i < static_cast<size_t>(argc); ++i) {
        const std::string_view arg = argv[i];

        if (arg == "-t") {
            sortMode = SortMode::Time;
        } else if (arg == "-S") {
            sortMode = SortMode::Size;
        }

        if (!knownFlags.contains(arg)) {
            paths.insert(arg);
        }
    }

    if (paths.empty()) {
        paths.insert(std::filesystem::current_path());
    }

    const bool longFormat = args.contains("-l");

    for (const auto& path : paths) {
        if (!std::filesystem::exists(path)) {
            std::println(std::cerr, "No such file or directory: {} ", path.string());
            continue;
        }

        if (std::filesystem::is_regular_file(path)) {
            std::print("{}", loadFileToString(path));
            continue;
        }

        std::vector<std::filesystem::directory_entry> entries;

        if (args.contains("-d")) {
            // List the directory itself, not its contents.
            entries.push_back(std::filesystem::directory_entry{path});
        } else if (args.contains("-R")) {
            for (const auto& entry : std::filesystem::recursive_directory_iterator{path}) {
                if (args.contains("-a") || !isHidden(entry.path())) {
                    entries.push_back(entry);
                }
            }
        } else {
            for (const auto& entry : std::filesystem::directory_iterator{path}) {
                if (args.contains("-a") || !isHidden(entry.path())) {
                    entries.push_back(entry);
                }
            }
        }

        if (sortMode == SortMode::Time) {
            std::ranges::sort(entries, [](const auto& a, const auto& b) {
                return a.last_write_time() > b.last_write_time();
            });
        } else if (sortMode == SortMode::Size) {
            std::ranges::sort(entries, [](const auto& a, const auto& b) {
                const auto sizeOf = [](const auto& e) {
                    return e.is_regular_file() ? e.file_size() : 0uz;
                };
                return sizeOf(a) > sizeOf(b);
            });
        }

        if (args.contains("-r")) {
            std::ranges::reverse(entries);
        }

        for (const auto& entry : entries) {
            if (longFormat) {
                std::println("{} {}", getFilePerms(entry.path(), entry.status()), entry.path().filename().string());
            } else {
                std::println("{}", entry.path().filename().string());
            }
        }
    }

    return 0;
}