#include <algorithm>
#include <cstddef>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <utility>
#include <vector>

static constexpr long long decryption_key = 811589153LL;

template<typename T>
void move_element(std::vector<T>& vec, long long pos, long long steps) {
    if (steps == 0)
        return;
    long long n = static_cast<long long>(vec.size()) - 1;
    T elem = vec[pos];
    vec.erase(vec.begin() + pos);
    long long new_pos = (pos + steps) % n + (pos + steps <= 0 ? n : 0);
    vec.insert(vec.begin() + new_pos, elem);
}

template<typename T>
T groove_sum(const std::vector<T>& encr, std::size_t iterations) {
    std::size_t n = encr.size();

    std::vector<std::pair<std::size_t, T>> vec;
    for (std::size_t i = 0; i < n; ++i) {
        vec.emplace_back(std::make_pair(i, encr[i]));
    }
    for (std::size_t k = 0; k < iterations; ++k) {
        for (std::size_t i = 0; i < n; ++i) {
            std::size_t pos = std::distance(vec.begin(), std::find_if(vec.begin(), vec.end(), [i](const auto& pair) -> bool {
                return pair.first == i;
            }));
            T steps = vec[pos].second;
            move_element(vec, pos, steps);
        }
    }
    std::vector<T> decr;
    std::transform(vec.begin(), vec.end(), std::back_inserter(decr), [](const auto& pair) -> T {
        return pair.second;
    });
    std::size_t idx_zero = std::distance(decr.begin(), std::find(decr.begin(), decr.end(), 0));

    T groove = 0;
    for (int i = 0; i < 3; i++) {
        T groove_coordinate = decr[(idx_zero + (i + 1) * 1000) % n];
        groove += groove_coordinate;
        std::cout << "Groove coordianate " << i + 1 << ": " << groove_coordinate << '\n';
    }
    return groove;
}

int main() {
    for (auto file : {"sample.txt", "input.txt"}) {
        std::ifstream input {file};
        if (!input) {
            std::cerr << "Failed to open: " << file << '\n';
            return 1;
        }
        std::vector<long long> encr;
        std::string line;
        while (std::getline(input, line)) {
            encr.push_back(std::stoi(line));
        }
        std::cout << file << ":\n";
        long long ans_one = groove_sum(encr, 1);
        std::cout << "Answer part 1:  " << ans_one << '\n';
        for (auto& x : encr) {
            x *= decryption_key;
        }
        long long ans_two = groove_sum(encr, 10);
        std::cout << "Answer part 2:  " << ans_two << "\n\n";
    }
    return 0;
}
