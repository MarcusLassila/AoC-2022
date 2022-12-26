#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>

namespace {
    const std::map<char, long long> snafu_digits = {{'=', -2}, {'-', -1}, {'0', 0}, {'1', 1}, {'2', 2}};
}

long long to_decimal(std::string_view snafu) {
    long long ret = 0;
    long long k = 1;
    for (auto it = snafu.crbegin(); it != snafu.crend(); ++it, k *= 5) {
        ret += k * snafu_digits.at(*it);
    }
    return ret;
}

std::string to_snafu(long long num) {
    std::stringstream ss;
    long long carry = 0;
    for (; num; num /= 5) {
        std::lldiv_t d = std::div(num % 5 + carry, 5LL);
        carry = d.quot;
        switch (d.rem) {
            case 0: case 1: case 2:
                ss << d.rem;
                break;
            case 3:
                ss << '=';
                ++carry;
                break;
            case 4:
                ss << '-';
                ++carry;
                break;
        }
    }
    std::string ret = ss.str();
    std::reverse(ret.begin(), ret.end());
    return ret;
}

int main() {
    for (auto file : {"sample.txt", "input.txt"}) {
        std::ifstream input {file};
        if (!input) {
            std::cerr << "Failed to open: " << file << '\n';
            return 1;
        }
        long long sum = 0;
        std::string line;
        while (std::getline(input, line)) {
            sum += to_decimal(line);
        }
        std::cout << file << ":\n";
        std::cout << "Answer:  " << to_snafu(sum) << '\n';
    }
    return 0;
}
