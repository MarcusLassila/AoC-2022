#include <array>
#include <fstream>
#include <iostream>
#include <numeric>
#include <string>
#include <utility>
#include <vector>

static const std::array<std::string, 2> input_files {"sample.txt", "input.txt"};

int part_one(const std::vector<std::pair<char, char>>& rounds) {
    return std::accumulate(rounds.cbegin(), rounds.cend(), 0, [](int score, const auto& round) -> int {
        const auto& [p, q] = round;
        return score + q - 'X' + 1 + (4 - (p - 'A' - q + 'X') % 3) % 3 * 3;
    });
}

int part_two(const std::vector<std::pair<char, char>>& rounds) {
    return std::accumulate(rounds.cbegin(), rounds.cend(), 0, [](int score, const auto& round) -> int {
        const auto& [p, q] = round;
        return score + 3 * (q - 'X') + (p - 'A' + q - 'Y' + 3) % 3 + 1;
    });
}

int main() {
    for (const auto& file : input_files) {
        std::ifstream input {file};
        if (!input) {
            std::cerr << "Failed to open: " << file << '\n';
            return 1;
        }
        std::string line;
        std::vector<std::pair<char, char>> rounds;
        while (std::getline(input, line)) {
            rounds.emplace_back(std::make_pair(line[0], line[2]));
        }
        std::cout << file << ":\n";
        std::cout << "Answer part 1:  " << part_one(rounds) << '\n';
        std::cout << "Answer part 2:  " << part_two(rounds) << '\n';
    }
    return 0;
}
