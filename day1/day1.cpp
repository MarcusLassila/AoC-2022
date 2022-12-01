#include <algorithm>
#include <array>
#include <functional>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

static const std::array<std::string, 2> input_files {"sample.txt", "input.txt"};

int main() {
    for (const auto& file : input_files) {
        std::ifstream input {file};
        if (!input) {
            std::cerr << "Failed to open: " << file << '\n';
            return 1;
        }
        std::vector<int> data = {0};
        std::string line;
        while (std::getline(input, line)) {
            if (!line.length())
                data.push_back(0);
            else
                data.back() += std::stoi(line);
        }
        std::sort(data.begin(), data.end(), std::greater<int>{});
        std::cout << file << ":\n";
        std::cout << "Answer part 1:  " << data[0] << '\n';
        std::cout << "Answer part 2:  " << data[0] + data[1] + data[2] << '\n';
    }
    return 0;
}
