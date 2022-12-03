#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>
#include <unordered_set>

int main() {
    for (auto file : {"sample.txt", "input.txt"}) {
        std::ifstream input {file};
        if (!input) {
            std::cerr << "Failed to open: " << file << '\n';
            return 1;
        }
        const auto score_item = [](char item) -> int {
            return item >= 'a' ? item - 'a' + 1 : item - 'A' + 27;
        };
        int score_one = 0;
        int score_two = 0;
        int line_num  = 0;
        std::unordered_map<char, int> counter;
        std::string line;
        while (std::getline(input, line)) {
            for (char x : std::unordered_set<char>{line.begin(), line.end()}) {
                ++counter[x];
            }
            unsigned int half_len = line.length() / 2;
            std::unordered_set<char> fst {line.begin(), line.begin() + half_len};
            char item = *std::find_if(line.begin() + half_len, line.end(), [&fst](char item) -> bool {
                return fst.count(item);
            });
            score_one += score_item(item);

            ++line_num;
            if (line_num % 3 == 0) {
                char badge = std::max_element(counter.begin(), counter.end(), [](const auto& a, const auto& b) -> bool {
                    return a.second < b.second;
                })->first;
                score_two += score_item(badge);
                counter.clear();
            }
        }
        std::cout << file << ":\n";
        std::cout << "Answer part 1:  " << score_one << '\n';
        std::cout << "Answer part 2:  " << score_two << '\n';
    }
    return 0;
}
