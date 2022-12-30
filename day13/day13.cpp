#include <algorithm>
#include <cstddef>
#include <fstream>
#include <iostream>
#include <iterator>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

struct RecursiveVector {
    std::optional<int> data;
    std::vector<RecursiveVector> subvector;

    RecursiveVector() {}
    RecursiveVector(int data) : data(std::make_optional<int>(data)) {}
    RecursiveVector(const std::vector<RecursiveVector>& subvector) : subvector(subvector) {}

    friend std::ostream& operator<<(std::ostream& os, const RecursiveVector& rv);
};

std::ostream& operator<<(std::ostream& os, const RecursiveVector& rv) {
    if (rv.data) {
        os << rv.data.value();
    } else {
        os << '[';
        for (std::size_t i = 0; i + 1 < rv.subvector.size(); ++i) {
            os << rv.subvector[i] << ',';
        }
        if (!rv.subvector.empty()) {
            os << rv.subvector.back();
        }
        os  << ']';
    }
    return os;
}

bool operator==(const RecursiveVector& a, const RecursiveVector& b) {
    if (a.data && b.data) {
        return a.data.value() == b.data.value();
    } else if (a.data) {
        auto wrapped = RecursiveVector{std::vector<RecursiveVector>{a.data.value()}};
        return wrapped == b;
    } else if (b.data) {
        auto wrapped = RecursiveVector{std::vector<RecursiveVector>{b.data.value()}};
        return a == wrapped;
    } else {
        std::size_t n = std::min(a.subvector.size(), b.subvector.size());
        for (std::size_t i = 0; i < n; ++i) {
            if (!(a.subvector[i] == b.subvector[i])) {
                return false;
            }
        }
        return a.subvector.size() == b.subvector.size();
    }
}

bool operator<(const RecursiveVector& a, const RecursiveVector& b) {
    if (a.data && b.data) {
        return a.data.value() < b.data.value();
    } else if (a.data) {
        auto wrapped = RecursiveVector{std::vector<RecursiveVector>{a.data.value()}};
        return wrapped < b;
    } else if (b.data) {
        auto wrapped = RecursiveVector{std::vector<RecursiveVector>{b.data.value()}};
        return a < wrapped;
    } else {
        std::size_t n = std::min(a.subvector.size(), b.subvector.size());
        for (std::size_t i = 0; i < n; ++i) {
            if (a.subvector[i] < b.subvector[i]) {
                return true;
            } else if (!(a.subvector[i] == b.subvector[i])) {
                return false;
            }
        }
        return a.subvector.size() < b.subvector.size();
    }
}

RecursiveVector parse_list(std::string_view list) {
    std::vector<RecursiveVector> stk = {RecursiveVector{}};
    std::string num;

    const auto push_num = [&stk](auto& num) -> void {
        if (!num.empty()) {
            stk.back().subvector.emplace_back(std::stoi(num));
            num.clear();
        }
    };

    for (char tok : list) {
        switch (tok) {
            case '[':
                stk.emplace_back();
                break;
            case ']':
                push_num(num);
                stk[stk.size() - 2].subvector.push_back(stk.back());
                stk.pop_back();
                break;
            case ',':
                push_num(num);
                break;
            default:
                num.push_back(tok);
                break;
        }
    }
    return stk.back().subvector.back();
}

int main() {
    for (auto file : {"sample.txt", "input.txt"}) {
        std::ifstream input {file};
        if (!input) {
            std::cerr << "Failed to open: " << file << '\n';
            return 1;
        }
        std::vector<RecursiveVector> rvs;
        std::string line;
        while (std::getline(input, line)) {
            if (line.empty()) {
                continue;  
            }
            rvs.push_back(parse_list(line));
        }
        unsigned int sum = 0;
        for (unsigned int i = 0; i < rvs.size(); i += 2) {
            if (rvs[i] < rvs[i + 1]) {
                sum += (i + 2) / 2;
            }
        }
        auto divider_packet_1 = RecursiveVector{std::vector<RecursiveVector>{RecursiveVector{2}}};
        auto divider_packet_2 = RecursiveVector{std::vector<RecursiveVector>{RecursiveVector{6}}};
        rvs.push_back(divider_packet_1);
        rvs.push_back(divider_packet_2);
        std::sort(rvs.begin(), rvs.end());
        auto idx_1 = std::distance(rvs.begin(), std::find(rvs.begin(), rvs.end(), divider_packet_1)) + 1;
        auto idx_2 = std::distance(rvs.begin(), std::find(rvs.begin(), rvs.end(), divider_packet_2)) + 1;
        std::cout << file << ":\n";
        std::cout << "Answer part 1:  " << sum << '\n';
        std::cout << "Answer part 2:  " << idx_1 * idx_2 << '\n';
    }
    return 0;
}
