#include <filesystem>
#include <print>

int main() {
    for (const auto& entry : std::filesystem::directory_iterator{"."}) {
        std::println("{}", entry.path().filename().string());
    }
    return 0;
}