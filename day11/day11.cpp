#include <algorithm>
#include <fstream>
#include <functional>
#include <iostream>
#include <queue>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

namespace {
    using ull = unsigned long long;
    constexpr std::string_view digits {"0123456789"};
    constexpr unsigned int num_rounds_one = 20U;
    constexpr unsigned int num_rounds_two = 10000U;
}

template<typename T>
struct Monkey {
    std::queue<T> items;
    std::function<T(T)> worry_update;
    std::function<int(T)> throw_to;
};

template<typename T>
T gcd(T a, T b) {
    return b == 0 ? a : gcd<T>(b, a % b);
}

template<typename T>
T lcm(T a, T b) {
    return a / gcd<T>(a, b) * b;
}

template<typename T>
ull monkey_business(unsigned int num_rounds, std::vector<Monkey<T>> monkeys, const std::function<T(T)>& worry_capper) {
    std::vector<ull> inspects(monkeys.size(), 0);
    for (unsigned int round = 0; round < num_rounds; ++round) {
        for (unsigned int i = 0; i < monkeys.size(); ++i) {
            Monkey<T>& monkey = monkeys[i];
            inspects[i] += monkey.items.size();
            while (!monkey.items.empty()) {
                T item = worry_capper(monkey.worry_update(monkey.items.front()));
                monkey.items.pop();
                monkeys[monkey.throw_to(item)].items.push(item);
            }
        }
    }
    std::sort(inspects.begin(), inspects.end(), std::greater<ull>{});
    return inspects[0] * inspects[1];
}

int main() {
    for (auto file : {"sample.txt", "input.txt"}) {
        std::ifstream input {file};
        if (!input) {
            std::cerr << "Failed to open: " << file << '\n';
            return 1;
        }
        std::string line;
        std::vector<Monkey<ull>> monkeys;
        ull mod = 1;
        while (std::getline(input, line)) {
            std::istringstream iss {line};
            std::string word;
            iss >> word;
            if (word == "Monkey") {
                monkeys.emplace_back(Monkey<ull>{});
            } else if (word == "Starting") {
                iss >> word;  // read with:
                ull x;
                std::queue<ull> items;
                while (iss >> x) {
                    monkeys.back().items.push(x);
                    iss >> word;  // read comma
                }
            } else if (word == "Operation:") {
                std::string l, r;
                char op;
                iss >> word >> word >> l >> op >> r;  // new = l op r
                monkeys.back().worry_update = [l, op, r](ull old) -> ull {
                    ull x = l == "old" ? old : std::stoull(l);
                    ull y = r == "old" ? old : std::stoull(r);
                    switch (op) {
                        case '+': return x + y;
                        case '-': return x - y;
                        case '*': return x * y;
                        case '/': return x / y;
                    }
                    return 0;
                };
            } else if (word == "Test:") {
                ull divby = std::stoull(std::string(line.begin() + line.find_first_of(digits), line.end()));

                std::getline(input, line);
                int idx_true = std::stoi(std::string(line.begin() + line.find_first_of(digits), line.end()));

                std::getline(input, line);
                int idx_false = std::stoi(std::string(line.begin() + line.find_first_of(digits), line.end()));

                mod = lcm<ull>(mod, divby);
                monkeys.back().throw_to = [divby, idx_true, idx_false](ull item) -> int {
                    return item % divby == 0 ? idx_true : idx_false;
                };
            }
        }
        ull ans_one = monkey_business<ull>(num_rounds_one, monkeys,    [](ull item) -> ull { return item / 3;   });
        ull ans_two = monkey_business<ull>(num_rounds_two, monkeys, [mod](ull item) -> ull { return item % mod; });
        std::cout << file << ":\n";
        std::cout << "Answer part 1:  " << ans_one << '\n';
        std::cout << "Answer part 2:  " << ans_two << '\n';
    }
    return 0;
}
