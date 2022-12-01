#include <bits/stdc++.h>

static const std::array<std::string, 2> input_files {"sample.txt", "input.txt"};

int main() {
    std::ifstream input;
    for (const auto& file : input_files) {
        input.open(file);
        if (!input) {
            std::cerr << "Failed to open: " << file << '\n';
            return 1;
        }
        std::string line;
        while (std::getline(input, line)) {
            std::cout << line << '\n';
        }
        std::cout << file << ":\n";
        std::cout << "Answer part 1:  " << "..." << '\n';
        std::cout << "Answer part 2:  " << "..." << '\n';
        input.close();
    }
    return 0;
}
