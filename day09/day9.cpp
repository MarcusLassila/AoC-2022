#include <array>
#include <cstddef>
#include <fstream>
#include <iostream>
#include <set>
#include <map>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace {
    constexpr unsigned int short_rope_length = 2U;
    constexpr unsigned int long_rope_length = 10U;
}

struct Point {
    int x, y;
    Point() : x(0), y(0) {}
    Point(int x, int y) : x(x), y(y) {}

    Point& operator+=(const Point& rhs);
    Point& operator-=(const Point& rhs);
    Point& operator*=(int rhs);
    friend std::ostream& operator<<(std::ostream& os, const Point& p);
};

Point& Point::operator+=(const Point& rhs) {
    x += rhs.x;
    y += rhs.y;
    return *this;
}

Point& Point::operator-=(const Point& rhs) {
    x += rhs.x;
    y += rhs.y;
    return *this;
}

Point& Point::operator*=(int rhs) {
    x += rhs;
    y += rhs;
    return *this;
}

std::ostream& operator<<(std::ostream& os, const Point& p) {
    os << '(' << p.x << ", " << p.y << ")";
    return os;
}

bool operator==(const Point& p, const Point& q) {
    return p.x == q.x && p.y == q.y;
}

bool operator!=(const Point& p, const Point& q) {
    return !(p == q);
}

bool operator<(const Point& p, const Point& q) {
    return p.x < q.x || (p.x == q.x && p.y < q.y);
}

bool operator<=(const Point& p, const Point& q) {
    return p == q || p < q;
}

bool operator>(const Point& p, const Point& q) {
    return !(p <= q);
}

bool operator>=(const Point& p, const Point& q) {
    return !(p < q);
}

Point operator+(const Point& p, const Point& q) {
    return {p.x + q.x, p.y + q.y};
}

Point operator-(const Point& p, const Point& q) {
    return {p.x - q.x, p.y - q.y};
}

Point operator*(int a, const Point& p) {
    return {a * p.x, a * p.y};
}

Point operator*(const Point& p, int a) {
    return {a * p.x, a * p.y};
}

int sign(int x) {
    return (x > 0) - (x < 0);
}

template<unsigned int rope_length>
std::size_t solve(const std::vector<std::pair<char, int>>& moves) {
    std::array<Point, rope_length> rope;
    std::set<Point> visited = {rope.front()};
    static const std::map<char, Point> directions = {{'U', {-1, 0}}, {'R', {0, 1}},
                                                     {'D', {1, 0}}, {'L', {0, -1}}};

    for (const auto& [dir, steps] : moves) {
        const auto move = directions.at(dir);
        for (int step = 0; step < steps; ++step) {
            rope.front() += move;
            for (unsigned int i = 0; i + 1 < rope_length; ++i) {
                auto diff = rope[i] - rope[i + 1];
                if (std::abs(diff.x) > 1 || std::abs(diff.y) > 1) {
                    rope[i + 1] += Point(sign(diff.x), sign(diff.y));
                }
            }
            visited.insert(rope.back());
        }
    }
    return visited.size();
}

int main() {
    for (auto file : {"sample.txt", "sample2.txt", "input.txt"}) {
        std::ifstream input {file};
        if (!input) {
            std::cerr << "Failed to open: " << file << '\n';
            return 1;
        }
        std::string line;
        std::vector<std::pair<char, int>> moves;
        while (std::getline(input, line)) {
            std::istringstream iss {line};
            char direction;
            int steps;
            iss >> direction >> steps;
            moves.emplace_back(direction, steps);
        }
        std::cout << file << ":\n";
        std::cout << "Answer part 1:  " << solve<short_rope_length>(moves) << '\n';
        std::cout << "Answer part 2:  " << solve<long_rope_length>(moves) << '\n';
    }
    return 0;
}
