#include <bits/stdc++.h>

int main() {
    for (auto file : {"sample.txt", "input.txt"}) {
        std::ifstream input {file};
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
    }
    return 0;
}
