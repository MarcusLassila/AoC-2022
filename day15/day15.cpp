#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>
#include <map>
#include <optional>
#include <set>
#include <sstream>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

namespace {
    using Interval = std::pair<int, int>;
    static constexpr long long tuning_multiplier = 4000000;
}

struct Point {
    int x, y;
    Point() : x(0), y(0) {}
    Point(int x, int y) : x(x), y(y) {}

    friend std::ostream& operator<<(std::ostream& os, const Point& p);
};

std::ostream& operator<<(std::ostream& os, const Point& p) {
    os << '(' << p.x << ',' << p.y << ')';
    return os;
}

unsigned int manhattan_dist(const Point& a, const Point& b) {
    return std::abs(a.x - b.x) + std::abs(a.y - b.y);
}

std::optional<Interval> find_excluded_interval(const std::pair<Point, Point>& scanner_beacon_pair, int row) {
    const auto& [scanner, beacon] = scanner_beacon_pair;
    auto man_dist = static_cast<int>(manhattan_dist(scanner, beacon));
    auto y_dist = std::abs(row - scanner.y);
    if (y_dist > man_dist) {
        return {};
    }
    int x_dist = man_dist - y_dist;
    int a = scanner.x - x_dist;
    int b = scanner.x + x_dist;
    return std::make_pair(a, b);
}

std::vector<Interval> merged_intervals(const std::vector<Interval>& intervals) {
    std::vector<Interval> merged;
    auto it = intervals.begin();
    while (it != intervals.end()) {
        auto curr = *it++;
        while (it != intervals.end() && it->first <= curr.second + 1) {
            curr.second = std::max(curr.second, it->second);
            ++it;
        }
        merged.push_back(curr);
    }
    return merged;
}

std::vector<Interval> excluded_intervals(int row, const std::vector<std::pair<Point, Point>>& scanner_beacon_pairs) {
    std::vector<Interval> excluded;
    for (const auto& pair : scanner_beacon_pairs) {
        auto ival = find_excluded_interval(pair, row);
        if (ival) {
            excluded.push_back(ival.value());
        }
    }
    std::sort(excluded.begin(), excluded.end(), [](const auto& a, const auto& b) -> bool {
        return a.first < b.first || (a.first == b.first && a.second < b.second);
    });
    return merged_intervals(excluded);
}

std::optional<int> find_gap(const std::vector<Interval>& excluded, int limit) {
    std::optional<int> gap;
    for (unsigned int i = 0; i + 1 < excluded.size(); ++i) {
        int a = excluded[i].second;
        int b = excluded[i + 1].first;
        if (a + 1 < b && 0 < b && a < limit) {
            gap = std::max(0, a + 1);
            break;
        }
    }
    return gap;
}

unsigned int exclusion_count(const std::vector<Interval>& excluded, const std::set<int>& beacons) {
    unsigned int count = 0;
    for (const auto& [a, b] : excluded) {
        count += b - a + 1;
        for (int x : beacons) {
            if (a <= x && x <= b) {
                --count;
            }
        }
    }
    return count;
}

long long tuning_frequency(int limit, const std::vector<std::pair<Point, Point>>& scanner_beacon_pairs) {
    long long freq = 0;
    for (int y = 0; y <= limit; ++y) {
        std::optional<int> x = find_gap(excluded_intervals(y, scanner_beacon_pairs), limit);
        if (x) {
            freq = x.value() * tuning_multiplier + y;
            break;
        }
    }
    return freq;
}

int main() {
    auto params_one = std::make_tuple("sample.txt", 10, 20);
    auto params_two = std::make_tuple("input.txt", 2000000, 4000000);
    for (const auto& [file, row, limit] : {params_one, params_two}) {
        std::ifstream input {file};
        if (!input) {
            std::cerr << "Failed to open: " << file << '\n';
            return 1;
        }
        std::vector<std::pair<Point, Point>> scanner_beacon_pairs;
        std::map<int, std::set<int>> beacon_map;
        std::string line;
        while (std::getline(input, line)) {
            line.erase(std::remove_if(line.begin(), line.end(), [](char tok) -> bool {
                return tok != '-' && !std::isspace(tok) && !std::isdigit(tok);
            }), line.end());
            std::istringstream iss {line};
            int x_a, y_a, x_b, y_b;
            iss >> x_a >> y_a >> x_b >> y_b;

            Point scanner {x_a, y_a};
            Point beacon  {x_b, y_b};
            scanner_beacon_pairs.emplace_back(scanner, beacon);
            beacon_map[beacon.y].insert(beacon.x);
        }
        std::vector<Interval> excluded = excluded_intervals(row, scanner_beacon_pairs);   
        std::cout << file << ":\n";
        std::cout << "Answer part 1:  " << exclusion_count(excluded, beacon_map.at(row)) << '\n';
        std::cout << "Answer part 2:  " << tuning_frequency(limit, scanner_beacon_pairs) << '\n';
    }
    return 0;
}
