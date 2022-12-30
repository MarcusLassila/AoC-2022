#include <algorithm>
#include <cctype>
#include <cstddef>
#include <fstream>
#include <iostream>
#include <iterator>
#include <sstream>
#include <string>
#include <tuple>
#include <vector>

using Stacks = std::vector<std::vector<char>>;
using Moves  = std::vector<std::tuple<int, int, int>>;

std::string top_string(const Stacks& stacks) {
    std::string ret;
    std::transform(stacks.cbegin(), stacks.cend(), std::back_inserter(ret), [](const auto& stk) -> char {
        return stk.empty() ? ' ' : stk.back();
    });
    return ret;
}

std::string solve(Stacks stacks, const Moves& moves, bool crate_mover_9001 = false) {
    for (const auto& [n, a, b] : moves) {
        if (crate_mover_9001)
            std::move(stacks[a].end() - n, stacks[a].end(), std::back_inserter(stacks[b]));
        else
            std::move(stacks[a].rbegin(), stacks[a].rbegin() + n, std::back_inserter(stacks[b]));
        stacks[a].erase(stacks[a].end() - n, stacks[a].end());
    }
    return top_string(stacks);
}

int main() {
    for (auto file : {"sample.txt", "input.txt"}) {
        std::ifstream input {file};
        if (!input) {
            std::cerr << "Failed to open: " << file << '\n';
            return 1;
        }
        std::size_t num_stacks = 0;
        std::string line;
        std::getline(input, line);
        num_stacks = (line.size() + 1) / 4;
        Stacks stacks(num_stacks);
        do {
            for (unsigned int i = 1; i < line.length(); i += 4) {
                if (std::isalpha(line[i])) {
                    stacks[(i - 1) / 4].push_back(line[i]);
                }
            }
        } while (std::getline(input, line) && !line.empty());

        for (auto& stk : stacks) {
            std::reverse(stk.begin(), stk.end());
        }
        Moves moves;
        while (std::getline(input, line)) {
            std::istringstream iss {line};
            std::string redundant;
            int n, a, b;
            iss >> redundant >> n >> redundant >> a >> redundant >> b;
            moves.emplace_back(n, a - 1, b - 1);
        }
        std::cout << file << ":\n";
        std::cout << "Answer part 1:  " << solve(stacks, moves, false) << '\n';
        std::cout << "Answer part 2:  " << solve(stacks, moves, true) << '\n';
    }
    return 0;
}
