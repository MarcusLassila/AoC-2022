#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

struct Point {
    int x, y;
    Point() : x(0), y(0) {}
    Point(int x, int y) : x(x), y(y) {}

    Point& operator+=(const Point& rhs);
    friend std::ostream& operator<<(std::ostream& os, const Point& p);
};

Point& Point::operator+=(const Point& rhs) {
    x += rhs.x;
    y += rhs.y;
    return *this;
}

std::ostream& operator<<(std::ostream& os, const Point& p) {
    os << '(' << p.x << ',' << p.y << ')';
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

Point operator+(const Point& p, const Point& q) {
    return Point(p.x + q.x, p.y + q.y);
}

Point operator-(const Point& p, const Point& q) {
    return Point(p.x - q.x, p.y - q.y);
}

template<>
struct std::hash<Point> {
    std::size_t operator()(Point const& p) const noexcept {
        std::size_t h1 = std::hash<int >{}(p.x);
        std::size_t h2 = std::hash<int >{}(p.y);
        return h1 ^ (h2 << 1);
    }
};

template<std::size_t I>
std::tuple_element_t<I, Point>& get(const Point& p) {
    if constexpr (I == 0) return p.x;
    if constexpr (I == 1) return p.y;
}

class AbstractMonkeyMap {
public:
    using Grid         = std::vector<std::string>;
    using Instructions = std::vector<std::string>;
    using Position     = std::pair<Point, int>;  // Coordinate and direction index
    AbstractMonkeyMap(const Grid& grid, const Instructions& instructions)
        : m_grid(grid)
        , m_instructions(instructions)
        {}
    char at(const Point& coord) const { return m_grid[coord.x][coord.y]; }
    virtual Position march(const Position& pos, int steps) const = 0;
    int trace_path() const;
protected:
    enum Direction {right, down, left, up};
    static const inline std::array<Point, 4> directions = {Point{0, 1}, Point{1, 0}, Point{0, -1}, Point{-1, 0}};
    Grid m_grid;
    Instructions m_instructions;
};

int AbstractMonkeyMap::trace_path() const {
    Position pos {Point{0, static_cast<int>(m_grid.front().find('.'))}, 0};
    for (const auto& inst : m_instructions) {
        if (inst == "L") {
            --pos.second;
            if (pos.second < 0)
                pos.second = 3;
        } else if (inst == "R") {
            ++pos.second;
            if (pos.second > 3)
                pos.second = 0;
        } else {
            pos = march(pos, std::stoi(inst));
        }
    }
    const auto& [coord, dir_idx] = pos;
    const auto& [row, col] = coord;
    return 1000 * (row + 1) + 4 * (col + 1) + dir_idx;
}

class MonkeyMap : public AbstractMonkeyMap {
public:
    MonkeyMap(const Grid& grid, const Instructions& instructions)
        : AbstractMonkeyMap{grid, instructions}
        , m_dimensions(std::make_pair(static_cast<int>(grid.size()),
                                      static_cast<int>(grid.front().size())))
        {}

    bool on_grid(const Point& coord) const;
    Position march(const Position& pos, int steps) const override;

private:
    std::pair<int, int> m_dimensions;
};

bool MonkeyMap::on_grid(const Point& coord) const {
    const auto& [m, n] = m_dimensions;
    const auto& [x, y] = coord;
    return 0 <= x && x < m && 0 <= y && y < n;
}

MonkeyMap::Position MonkeyMap::march(const MonkeyMap::Position& pos, int steps) const {
    Point coord = pos.first;
    const int dir_idx = pos.second;

    const auto toroidal_step = [this, dir_idx](const Point& coord) -> Point {
        const auto& [x, y]   = coord;
        const auto& [M, N]   = m_dimensions;
        const auto& [dx, dy] = directions[dir_idx];
        return {((x + dx) % M + M) % M, ((y + dy) % N + N) % N};
    };

    Point curr = coord;
    while (steps > 0) {
        Point next = toroidal_step(curr);
        while (at(next) == ' ') {
            next = toroidal_step(next);
        }
        switch (at(next)) {
            case '.':
                --steps;
                break;
            case '#':
                return std::make_pair(curr, dir_idx);
        }
        curr = next;
    }
    return std::make_pair(curr, dir_idx);
}

class MonkeyCube : public AbstractMonkeyMap {
public:
    MonkeyCube(const Grid& grid, const Instructions& instructions)
        : AbstractMonkeyMap{grid, instructions}
        , m_edge_connections(cube_edge_connections(grid))
        {}

    Position march(const Position& pos, int steps) const override;

private:
    std::map<Position, Position> m_edge_connections;
    std::map<Position, Position> cube_edge_connections(const Grid&) const;
};

MonkeyCube::Position MonkeyCube::march(const Position& pos, int steps) const {
    Position curr = pos;
    while (steps > 0) {
        auto it = m_edge_connections.find(curr);
        Position next = it != m_edge_connections.end()
                        ? it->second
                        : std::make_pair(curr.first + directions[curr.second], curr.second);
        switch (at(next.first)) {
            case '.':
                --steps;
                break;
            case '#':
                return curr;
        }
        curr = next;
    }
    return curr;
}

std::map<MonkeyCube::Position, MonkeyCube::Position> MonkeyCube::cube_edge_connections(const MonkeyCube::Grid& grid) const {
    std::map<MonkeyCube::Position, MonkeyCube::Position> edge_connections;

    std::size_t N = 0;
    for (const auto& row : grid) {
        N += std::count_if(row.begin(), row.end(), [](const char x) -> bool {
            return x == '.' || x == '#';
        });
    }
    int n = static_cast<int>(std::sqrt(N / 6));

    std::string shape;
    for (std::size_t i = 0; i < grid.size(); i += n) {
        for (std::size_t j = 0; j < grid[i].size(); j += n) {
            if (grid[i][j] == '.' || grid[i][j] == '#') {
                shape.push_back('#');
            } else {
                shape.push_back(' ');
            }
        }
        shape.push_back('\n');
    }

    if (shape == "  #\n###\n  ##\n") {

        Point a, b;
        for (int i = 0; i < n; ++i) {
            a = Point{0, i + 2 * n};
            b = Point{n, n - 1 - i};
            edge_connections[std::make_pair(a, up)] = std::make_pair(b, down);
            edge_connections[std::make_pair(b, up)] = std::make_pair(a, down);

            a = Point{i, 3 * n - 1};
            b = Point{3 * n - 1 - i, 4 * n - 1};
            edge_connections[std::make_pair(a, right)] = std::make_pair(b, left);
            edge_connections[std::make_pair(b, right)] = std::make_pair(a, left);

            a = Point{n + i, 3 * n - 1};
            b = Point{2 * n, 4 * n - 1 - i};
            edge_connections[std::make_pair(a, right)] = std::make_pair(b, down);
            edge_connections[std::make_pair(b, up)]    = std::make_pair(a, left);

            a = Point{3 * n - 1, 3 * n + i};
            b = Point{2 * n - 1 - i, 0};
            edge_connections[std::make_pair(a, down)] = std::make_pair(b, right);
            edge_connections[std::make_pair(b, left)] = std::make_pair(a, up);

            a = Point{3 * n - 1, 2 * n + i};
            b = Point{2 * n - 1, n - 1 - i};
            edge_connections[std::make_pair(a, down)] = std::make_pair(b, up);
            edge_connections[std::make_pair(b, down)] = std::make_pair(a, up);

            a = Point{2 * n, 2 * n + i};
            b = Point{2 * n - 1, 2 * n - 1 - i};
            edge_connections[std::make_pair(a, left)] = std::make_pair(b, up);
            edge_connections[std::make_pair(b, down)] = std::make_pair(a, right);

            a = Point{n, n + i};
            b = Point{i, 2 * n};
            edge_connections[std::make_pair(a, up)]   = std::make_pair(b, right);
            edge_connections[std::make_pair(b, left)] = std::make_pair(a, down);
        }

    } else if (shape == " ##\n #\n##\n#\n") {

        Point a, b;
        for (int i = 0; i < n; ++i) {
            a = Point{0, n + i};
            b = Point{3 * n + i, 0};
            edge_connections[std::make_pair(a, up)]   = std::make_pair(b, right);
            edge_connections[std::make_pair(b, left)] = std::make_pair(a, down);

            a = Point{0, 2 * n + i};
            b = Point{4 * n - 1, i};
            edge_connections[std::make_pair(a, up)]   = std::make_pair(b, up);
            edge_connections[std::make_pair(b, down)] = std::make_pair(a, down);

            a = Point{i, 3 * n - 1};
            b = Point{3 * n - 1 - i, 2 * n - 1};
            edge_connections[std::make_pair(a, right)] = std::make_pair(b, left);
            edge_connections[std::make_pair(b, right)] = std::make_pair(a, left);

            a = Point{n - 1, 2 * n + i};
            b = Point{n + i, 2 * n - 1};
            edge_connections[std::make_pair(a, down)]  = std::make_pair(b, left);
            edge_connections[std::make_pair(b, right)] = std::make_pair(a, up);

            a = Point{3 * n - 1, n + i};
            b = Point{3 * n + i, n - 1};
            edge_connections[std::make_pair(a, down)]  = std::make_pair(b, left);
            edge_connections[std::make_pair(b, right)] = std::make_pair(a, up);

            a = Point{2 * n + i, 0};
            b = Point{n - 1 - i, n};
            edge_connections[std::make_pair(a, left)] = std::make_pair(b, right);
            edge_connections[std::make_pair(b, left)] = std::make_pair(a, right);

            a = Point{2 * n, i};
            b = Point{n + i, n};
            edge_connections[std::make_pair(a, up)]   = std::make_pair(b, right);
            edge_connections[std::make_pair(b, left)] = std::make_pair(a, down);
        }

    } else {
        throw std::runtime_error("Unimplemented shape");
    }

    return edge_connections;
}

int part_one(std::vector<std::string> grid, const std::vector<std::string>& instructions) {
    std::size_t longest_row = std::max_element(grid.begin(), grid.end(), [](const auto& a, const auto& b) -> bool {
        return a.size() < b.size();
    })->size();
    for (auto& row : grid) {
        if (row.size() < longest_row) {
            auto fill = std::string(longest_row - row.size(), ' ');
            row.insert(row.end(), fill.begin(), fill.end());
        }
    }
    return MonkeyMap{grid, instructions}.trace_path();
}

int part_two(const std::vector<std::string>& grid, const std::vector<std::string>& instructions) {
    return MonkeyCube{grid, instructions}.trace_path();
}

int main() {
    for (auto file : {"sample.txt", "input.txt"}) {
        std::ifstream input {file};
        if (!input) {
            std::cerr << "Failed to open: " << file << '\n';
            return 1;
        }
        std::vector<std::string> grid;
        std::string line;
        while (std::getline(input, line) && !line.empty()) {
            grid.push_back(line);
        }

        std::string inst_str;
        std::getline(input, inst_str);
        std::stringstream ss;
        for (std::size_t i = 0; i + 1 < inst_str.size(); ++i) {
            ss << inst_str[i];
            if (!std::isdigit(inst_str[i]) || !std::isdigit(inst_str[i + 1])) {
                ss << ' ';
            }
        }
        ss << inst_str.back();
        std::istringstream iss {ss.str()};
        std::string inst;
        std::vector<std::string> instructions;
        while (iss >> inst) {
            instructions.push_back(inst);
        }

        int ans_one = part_one(grid, instructions);
        int ans_two = part_two(grid, instructions);

        std::cout << file << ":\n";
        std::cout << "Answer part 1:  " << ans_one << '\n';
        std::cout << "Answer part 2:  " << ans_two << '\n';
    }
    return 0;
}
