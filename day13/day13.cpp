#include <algorithm>
#include <fstream>
#include <iostream>
#include <iterator>
#include <memory>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

struct RecursiveVector {
    std::optional<int> data;
    std::vector<RecursiveVector> vec;
    RecursiveVector() {}
    RecursiveVector(int data) : data(std::make_optional<int>(data)) {}

    friend std::ostream& operator<<(std::ostream& os, const RecursiveVector& rv);
};

std::ostream& operator<<(std::ostream& os, const RecursiveVector& rv) {
    if (rv.data) {
        os << rv.data.value();
    } else {
        os << '[';
        for (std::size_t i = 0; i + 1 < rv.vec.size(); ++i) {
            os << rv.vec[i] << ',';
        }
        if (!rv.vec.empty()) {
            os << rv.vec.back();
        }
        os  << ']';
    }
    return os;
}

bool operator==(const RecursiveVector& a, const RecursiveVector& b) {
    if (a.data && b.data) {
        return a.data.value() == b.data.value();
    } else if (a.data) {
        RecursiveVector wrapper;
        wrapper.vec.emplace_back(RecursiveVector{a.data.value()});
        return wrapper == b;
    } else if (b.data) {
        RecursiveVector wrapper;
        wrapper.vec.emplace_back(RecursiveVector{b.data.value()});
        return a == wrapper;
    } else {
        std::size_t n = std::min(a.vec.size(), b.vec.size());
        for (std::size_t i = 0; i < n; ++i) {
            if (!(a.vec[i] == b.vec[i])) {
                return false;
            }
        }
        return a.vec.size() == b.vec.size();
    }
}

bool operator<(const RecursiveVector& a, const RecursiveVector& b) {
    if (a.data && b.data) {
        return a.data.value() < b.data.value();
    } else if (a.data) {
        RecursiveVector wrapper;
        wrapper.vec.emplace_back(RecursiveVector{a.data.value()});
        return wrapper < b;
    } else if (b.data) {
        RecursiveVector wrapper;
        wrapper.vec.emplace_back(RecursiveVector{b.data.value()});
        return a < wrapper;
    } else {
        std::size_t n = std::min(a.vec.size(), b.vec.size());
        for (std::size_t i = 0; i < n; ++i) {
            if (a.vec[i] < b.vec[i]) {
                return true;
            } else if (!(a.vec[i] == b.vec[i])) {
                return false;
            }
        }
        return a.vec.size() < b.vec.size();
    }
}

RecursiveVector parse_list(std::string_view list) {
    std::vector<RecursiveVector> stk = {RecursiveVector{}};
    std::string num;

    for (char tok : list) {
        switch (tok) {
            case '[':
                stk.emplace_back(RecursiveVector{});
                break;
            case ']':
                if (!num.empty()) {
                    stk.back().vec.emplace_back(RecursiveVector{std::stoi(num)});
                    num.clear();
                }
                stk[stk.size() - 2].vec.push_back(stk.back());
                stk.pop_back();
                break;
            case ',':
                if (!num.empty()) {
                    stk.back().vec.emplace_back(RecursiveVector{std::stoi(num)});
                    num.clear();
                }
                break;
            default:
                num.push_back(tok);
                break;
        }
    }
    return stk.back().vec.back();
}

int main() {
    for (auto file : {"sample.txt", "input.txt"}) {
        std::ifstream input {file};
        if (!input) {
            std::cerr << "Failed to open: " << file << '\n';
            return 1;
        }
        std::vector<RecursiveVector> lists;
        std::string line;
        for (int i = 0; std::getline(input, line); ++i) {
            if (i % 3 == 2) {
                continue;  
            }
            lists.emplace_back(parse_list(line));
        }
        unsigned int sum = 0;
        for (std::size_t i = 0; i < lists.size(); i += 2) {
            if (lists[i] < lists[i + 1]) {
                sum += (i + 2) / 2;
            }
        }
        RecursiveVector dp1;
        RecursiveVector dp2;
        dp1.vec.emplace_back(RecursiveVector{});
        dp2.vec.emplace_back(RecursiveVector{});
        dp1.vec.back().vec.emplace_back(RecursiveVector{2});
        dp2.vec.back().vec.emplace_back(RecursiveVector{6});
        lists.push_back(dp1);
        lists.push_back(dp2);
        std::sort(lists.begin(), lists.end());
        std::size_t idx1 = 0, idx2 = 0;
        for (std::size_t k = 0; k < lists.size(); k++) {
            if (lists[k] == dp1) {
                idx1 = k + 1;
            } else if (lists[k] == dp2) {
                idx2 = k + 1;
            }
        }
        std::cout << file << ":\n";
        std::cout << "Answer part 1:  " << sum << '\n';
        std::cout << "Answer part 2:  " << idx1 * idx2 << '\n';
    }
    return 0;
}
