#include <fstream>
#include <iostream>
#include <regex>
#include <string>
#include <utility>

bool is_fully_overlapping(const std::pair<int, int>& range_a, const std::pair<int, int>& range_b) {
    const auto& [a, b] = range_a;
    const auto& [c, d] = range_b;
    return (a >= c && b <= d) || (c >= a && d <= b);
}

bool is_overlapping(const std::pair<int, int>& range_a, const std::pair<int, int>& range_b) {
    const auto& [a, b] = range_a;
    const auto& [c, d] = range_b;
    return a <= d && b >= c;
}

int main() {
    std::regex re {"(\\d+)-(\\d+),(\\d+)-(\\d+)"};
    std::smatch sm;
    for (auto file : {"sample.txt", "input.txt"}) {
        std::ifstream input {file};
        if (!input) {
            std::cerr << "Failed to open: " << file << '\n';
            return 1;
        }
        int ans_one = 0;
        int ans_two = 0;
        std::string line;
        while (std::getline(input, line)) {
            std::regex_match(line, sm, re);
            auto range_a = std::make_pair(std::stoi(sm[1]), std::stoi(sm[2]));
            auto range_b = std::make_pair(std::stoi(sm[3]), std::stoi(sm[4]));
            ans_one += is_fully_overlapping(range_a, range_b);
            ans_two += is_overlapping(range_a, range_b);
        }
        std::cout << file << ":\n";
        std::cout << "Answer part 1:  " << ans_one << '\n';
        std::cout << "Answer part 2:  " << ans_two << '\n';
    }
    return 0;
}
