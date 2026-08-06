#include <filesystem>
#include <fstream>
#include <print>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_set>

const std::unordered_set<std::string_view> knownFlags = {"-a", "-l"};

[[nodiscard]] static bool isHidden(const std::filesystem::path& path) {
    // Only really works on POSIX for now
    const std::string filename = path.filename().string();
    return !filename.empty() && filename.at(0) == '.';
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

int main(int argc, char** argv) {
    std::unordered_set<std::string_view> args(argv, argv + argc);
    std::unordered_set<std::filesystem::path> paths;

    for (size_t i{1uz}; i < static_cast<size_t>(argc); ++i) {
        if (!knownFlags.contains(argv[i])) {
            paths.insert(argv[i]);
        }
    }
    if (paths.empty()) {
        paths.insert(std::filesystem::current_path());
    }

    for (const auto& path : paths) {
        if (!std::filesystem::exists(path)) {
            std::println(std::cerr, "No such file or directory: {} ", path.string());
            continue;
        }

        if (std::filesystem::is_regular_file(path)) {
            std::print("{}", loadFileToString(path));
            continue;
        }

        for (const auto& entry : std::filesystem::directory_iterator{path}) {
            if (args.contains("-a") || !isHidden(entry.path())) {
                std::println("{}", entry.path().filename().string());
            }
        }
    }

    return 0;
}