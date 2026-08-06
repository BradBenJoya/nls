#include <filesystem>
#include <print>
#include <iostream>
#include <string>

[[nodiscard]] bool isHidden(const std::filesystem::path& path) {
    // Only really works on POSIX for now
    const std::string filename = path.filename().string();
    return !filename.empty() && filename.at(0) == '.';
}

int main(int argc, char** argv) {
    std::filesystem::path path = (argc > 1) ? argv[1] : std::filesystem::current_path();
    if (!std::filesystem::exists(path)) {
        std::println(std::cerr, "No such file or directory: {} ", path.string());
        std::exit(1);
    }

    for (const auto& entry : std::filesystem::directory_iterator{path}) {
        if (!isHidden(entry.path())) {
            std::println("{}", entry.path().filename().string());
        }
    }

    return 0;
}