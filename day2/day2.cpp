#include <array>
#include <fstream>
#include <iostream>
#include <string>

static const std::array<std::string, 2> input_files {"sample.txt", "input.txt"};

int main() {
    for (const auto& file : input_files) {
        std::ifstream input {file};
        if (!input) {
            std::cerr << "Failed to open: " << file << '\n';
            return 1;
        }
        int score_one = 0;
        int score_two = 0;
        std::string line;
        while (std::getline(input, line)) {
            char p = line[0];
            char q = line[2];
            score_one += q - 'X' + 1 + (4 - (p - 'A' - q + 'X') % 3) % 3 * 3;
            score_two += 3 * (q - 'X') + (p - 'A' + q - 'Y' + 3) % 3 + 1;
        }
        std::cout << file << ":\n";
        std::cout << "Answer part 1:  " << score_one << '\n';
        std::cout << "Answer part 2:  " << score_two << '\n';
    }
    return 0;
}
