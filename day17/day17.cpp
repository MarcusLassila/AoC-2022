#include <algorithm>
#include <array>
#include <fstream>
#include <iostream>
#include <set>
#include <string>
#include <string_view>
#include <utility>

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

struct Rock {
    std::set<Point> points;
    Rock() {}
    Rock(const std::set<Point>& points) : points(points) {}

    Rock move_up(int steps = 1) const;
    Rock move_down(int steps = 1) const;
    Rock move_left(int steps = 1) const;
    Rock move_right(int steps = 1) const;

    int top() const;
    int bottom() const;
    int left_side() const;
    int right_side() const;
};

Rock Rock::move_up(int steps) const {
    Rock moved_rock;
    for (const auto& p : points) {
        moved_rock.points.insert({p.x - steps, p.y});
    }
    return moved_rock;
}

Rock Rock::move_down(int steps) const {
    Rock moved_rock;
    for (const auto& p : points) {
        moved_rock.points.insert({p.x + steps, p.y});
    }
    return moved_rock;
}

Rock Rock::move_left(int steps) const {
    Rock moved_rock;
    for (const auto& p : points) {
        moved_rock.points.insert({p.x, p.y - steps});
    }
    return moved_rock;
}

Rock Rock::move_right(int steps) const {
    Rock moved_rock;
    for (const auto& p : points) {
        moved_rock.points.insert({p.x, p.y + steps});
    }
    return moved_rock;
}

int Rock::top() const {
    return std::min_element(points.begin(), points.end(), [](const auto& p, const auto& q) -> bool {
        return p.x < q.x;
    })->x;
}

int Rock::bottom() const {
    return std::max_element(points.begin(), points.end(), [](const auto& p, const auto& q) -> bool {
        return p.x < q.x;
    })->x;
}

int Rock::left_side() const {
    return std::min_element(points.begin(), points.end(), [](const auto& p, const auto& q) -> bool {
        return p.y < q.y;
    })->y;
}

int Rock::right_side() const {
    return std::max_element(points.begin(), points.end(), [](const auto& p, const auto& q) -> bool {
        return p.y < q.y;
    })->y;
}

bool intersects(const std::set<Point>& u, const std::set<Point>& v) {
    if (u.size() > v.size()) {
        return intersects(v, u);
    }
    return std::any_of(u.cbegin(), u.cend(), [&v](const auto& p) -> bool {
        return v.count(p);
    });
}

namespace {
    constexpr int simulator_width = 7;
}

class Simulator {
public:
    Simulator(std::string_view moves) : m_moves(moves) { find_cycle(); }

    void reset();
    void drop(const Rock& rock);
    void drop_rocks(unsigned int num_rocks);
    void find_cycle();

    int height() const { return -m_top; }
    bool is_occupied(const Point& p) const { return m_occupied.count(p); }
    std::pair<unsigned int, unsigned int> periodicity() const { return {m_cycle_start_idx, m_cycle_length}; }

    friend std::ostream& operator<<(std::ostream&, const Simulator&);

private:
    std::string m_moves;
    std::set<Point> m_occupied;
    int m_top = 0;
    unsigned int m_move_idx = 0;
    unsigned int m_cycle_start_idx;
    unsigned int m_cycle_length;
    static const inline std::array<Rock, 5> rocks = {
        std::set<Point>{{0, 2}, {0, 3}, {0, 4}, {0, 5}},
        std::set<Point>{{0, 3}, {-1, 2}, {-1, 3}, {-1, 4}, {-2, 3}},
        std::set<Point>{{0, 2}, {0, 3}, {0, 4}, {-1, 4}, {-2, 4}},
        std::set<Point>{{0, 2}, {-1, 2}, {-2, 2}, {-3, 2}},
        std::set<Point>{{0, 2}, {0, 3}, {-1, 2}, {-1, 3}},
    };
    std::array<int, 7> relative_levels() const;
};

void Simulator::reset() {
    m_occupied.clear();
    m_top = 0;
    m_move_idx = 0;
}

void Simulator::drop_rocks(unsigned int num_rocks) {
    for (unsigned int i = 0; i < num_rocks; ++i) {
        drop(rocks[i % rocks.size()]);
    }
}

void Simulator::drop(const Rock& rock_inp) {
    Rock rock = rock_inp.move_up(3 - m_top);
    while (true) {
        switch (m_moves[m_move_idx]) {
            case '<':
                if (rock.left_side() > 0) {
                    rock = rock.move_left();
                    if (intersects(rock.points, m_occupied)) {
                        rock = rock.move_right();
                    }
                }
                break;
            case '>':
                if (rock.right_side() + 1 < simulator_width) {
                    rock = rock.move_right();
                    if (intersects(rock.points, m_occupied)) {
                        rock = rock.move_left();
                    }
                }
                break;
        }
        ++m_move_idx;
        if (m_move_idx == m_moves.size()) {
            m_move_idx = 0;
        }
        rock = rock.move_down();
        if (intersects(rock.points, m_occupied) || rock.bottom() == 1) {
            rock  = rock.move_up();
            m_top = std::min(rock.top() - 1, m_top);
            m_occupied.insert(rock.points.begin(), rock.points.end());
            break;
        }
    }
}

void Simulator::find_cycle() {
    std::set<std::array<int, simulator_width + 2>> seen;  // Rock index, move index and relative height levels
    Point start_indices;
    unsigned int i = 0;
    while (true) {
        drop(rocks[i % rocks.size()]);
        std::array<int, 9> key;
        key[0] = static_cast<int>(i % rocks.size());
        key[1] = static_cast<int>(m_move_idx);
        std::array<int, 7> levels = relative_levels();
        for (int k = 0; k < simulator_width; ++k) {
            key[k + 2] = levels[k];
        }
        if (key[0] == 0 && seen.count(key)) {
            m_cycle_start_idx = i;
            start_indices = Point(key[0], key[1]);
            break;
        }
        seen.insert(key);
        ++i;
    }
    ++i;
    m_cycle_length = 1;
    while (true) {
        drop(rocks[i % rocks.size()]);
        Point indices {static_cast<int>(i % rocks.size()),
                       static_cast<int>(m_move_idx)};
        if (indices == start_indices) {
            break;
        }
        ++m_cycle_length;
        ++i;
    }
    m_cycle_start_idx -= m_cycle_length;
    reset();
}

/* Used for finding the cycle */
std::array<int, 7> Simulator::relative_levels() const {
    std::array<int, simulator_width> levels;
    levels.fill(0);
    for (const auto& p : m_occupied) {
        levels[p.y] = std::min(levels[p.y], p.x);
    }
    for (auto& lvl :levels) {
        lvl -= m_top;
    }
    return levels;
}

std::ostream& operator<<(std::ostream& os, const Simulator& sim) {
    for (int x = sim.height(); x >= 0; --x) {
        auto s = std::to_string(x);
        os << s << std::string(3 - s.size(), ' ') << '|';
        for (int y = 0; y < simulator_width; ++y) {
            os << (sim.is_occupied({-x, y}) ? '#' : '.');
        }
        os << '\n';
    }
    return os;
}

unsigned long long compute_height(std::string_view moves, unsigned long long drops) {
    Simulator sim {moves};
    const auto& [start, len] = sim.periodicity();
    if (drops <= start + len) {
        sim.drop_rocks(static_cast<unsigned int>(drops));
        return sim.height();
    }

    sim.drop_rocks(start);
    int ref = sim.height();
    sim.drop_rocks(len);
    int diff = sim.height() - ref;

    drops -= start + len;
    unsigned long long quot = drops / len;
    unsigned int rem = static_cast<unsigned int>(drops % len);

    sim.drop_rocks(rem);
    return sim.height() + quot * diff;
}

int main() {
    for (auto file : {"sample.txt", "input.txt"}) {
        std::ifstream input {file};
        if (!input) {
            std::cerr << "Failed to open: " << file << '\n';
            return 1;
        }
        std::string moves;
        std::getline(input, moves);

        Simulator sim {moves};

        sim.drop_rocks(10);
        std::cout << "First 10 blocks dropped:\n";
        std::cout << sim << '\n';
        sim.reset();

        unsigned long long ans_one = compute_height(moves, 2022);
        unsigned long long ans_two = compute_height(moves, 1000000000000);

        std::cout << file << ":\n";
        std::cout << "Answer part 1:  " << ans_one << '\n';
        std::cout << "Answer part 2:  " << ans_two << "\n\n";
    }
    return 0;
}
