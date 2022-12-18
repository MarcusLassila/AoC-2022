#include <bits/stdc++.h>

#define Int long long

struct Point {
    Int x, y;
    Point() : x(0), y(0) {}
    Point(Int x, Int y) : x(x), y(y) {}

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
struct std::hash<Point> {  // https://en.cppreference.com/w/cpp/utility/hash
    std::size_t operator()(Point const& p) const noexcept {
        std::size_t h1 = std::hash<Int >{}(p.x);
        std::size_t h2 = std::hash<Int >{}(p.y);
        return h1 ^ (h2 << 1);
    }
};

struct Rock {
    std::set<Point> points;
    Rock() {}
    Rock(const std::set<Point>& points) : points(points) {}

    Rock move_up(Int steps = 1) const;
    Rock move_down(Int steps = 1) const;
    Rock move_left(Int steps = 1) const;
    Rock move_right(Int steps = 1) const;

    Int top() const;
    Int bottom() const;
    Int left_side() const;
    Int right_side() const;
};

Rock Rock::move_up(Int steps) const {
    Rock moved_rock;
    for (const auto& p : points) {
        moved_rock.points.insert({p.x - steps, p.y});
    }
    return moved_rock;
}

Rock Rock::move_down(Int steps) const {
    Rock moved_rock;
    for (const auto& p : points) {
        moved_rock.points.insert({p.x + steps, p.y});
    }
    return moved_rock;
}

Rock Rock::move_left(Int steps) const {
    Rock moved_rock;
    for (const auto& p : points) {
        moved_rock.points.insert({p.x, p.y - steps});
    }
    return moved_rock;
}

Rock Rock::move_right(Int steps) const {
    Rock moved_rock;
    for (const auto& p : points) {
        moved_rock.points.insert({p.x, p.y + steps});
    }
    return moved_rock;
}

Int Rock::top() const {
    return std::min_element(points.begin(), points.end(), [](const auto& p, const auto& q) -> bool {
        return p.x < q.x;
    })->x;
}

Int Rock::bottom() const {
    return std::max_element(points.begin(), points.end(), [](const auto& p, const auto& q) -> bool {
        return p.x < q.x;
    })->x;
}

Int Rock::left_side() const {
    return std::min_element(points.begin(), points.end(), [](const auto& p, const auto& q) -> bool {
        return p.y < q.y;
    })->y;
}

Int Rock::right_side() const {
    return std::max_element(points.begin(), points.end(), [](const auto& p, const auto& q) -> bool {
        return p.y < q.y;
    })->y;
}

static const std::vector<Rock> rocks = {
    std::set<Point>{{0, 2}, {0, 3}, {0, 4}, {0, 5}},
    std::set<Point>{{0, 3}, {-1, 2}, {-1, 3}, {-1, 4}, {-2, 3}},
    std::set<Point>{{0, 2}, {0, 3}, {0, 4}, {-1, 4}, {-2, 4}},
    std::set<Point>{{0, 2}, {-1, 2}, {-2, 2}, {-3, 2}},
    std::set<Point>{{0, 2}, {0, 3}, {-1, 2}, {-1, 3}},
};

void print(const std::set<Point>& occupied, Int high_point) {
    for (Int x = high_point; x <= 0; ++x) {
        std::string s = std::to_string(-x);
        std::cout << s << std::string(3 - s.size(), ' ') << '|';
        for (Int y = 0; y < 7; ++y) {
            if (occupied.count({x, y})) {
                std::cout << '#';
            } else {
                std::cout << '.';
            }
        }
        std::cout << '\n';
    }
}

bool intersects(const std::set<Point>& u, const std::set<Point>& v) {
    if (u.size() > v.size()) {
        return intersects(v, u);
    }
    return std::any_of(u.cbegin(), u.cend(), [&v](const auto& p) -> bool {
        return v.count(p);
    });
}

void iterate(std::string_view moves, Rock rock, unsigned int& move_idx, std::set<Point>& occupied, Int& high_point) {
    rock = rock.move_up(3 - high_point);
    while (true) {
        switch (moves[move_idx]) {
            case '<':
                if (rock.left_side() > 0) {
                    rock = rock.move_left();
                    if (intersects(rock.points, occupied)) {
                        rock = rock.move_right();
                    }
                }
                break;
            case '>':
                if (rock.right_side() < 6) {
                    rock = rock.move_right();
                    if (intersects(rock.points, occupied)) {
                        rock = rock.move_left();
                    }
                }
                break;
        }
        ++move_idx;
        if (move_idx == moves.size()) {
            move_idx = 0;
        }
        rock = rock.move_down();
        if (intersects(rock.points, occupied) || rock.bottom() == 1) {
            rock = rock.move_up();
            high_point = std::min(rock.top() - 1, high_point);
            occupied.insert(rock.points.begin(), rock.points.end());
            break;
        }
    }
}

Int run_iterations(unsigned int iterations, std::string_view moves, unsigned int& move_idx, std::set<Point>& occupied, Int high_point) {
    for (unsigned Int i = 0; i < iterations; ++i) {
        Rock rock = rocks[i % rocks.size()];
        iterate(moves, rock, move_idx, occupied, high_point);
        // if (i == 13 || i == 14 || i == 15 || i == 16) {
        //     print(occupied, high_point);
        //     std::cout << '\n';
        // }
    }
    return high_point;
}

Int part_one(std::string_view moves) {
    Int high_point = 0;
    unsigned int move_idx = 0;
    std::set<Point> occupied;
    return -run_iterations(2022, moves, move_idx, occupied, high_point);
}

std::array<Int, 7> bottom_scan(const std::set<Point>& occupied, Int high_point) {
    std::array<Int, 7> levels;
    levels.fill(0);
    for (const auto& p : occupied) {
        levels[p.y] = std::min(levels[p.y], p.x);
    }
    for (int k = 0; k < 7; ++k) {
        levels[k] = levels[k] - high_point;
    }
    //std::cout << std::endl;
    return levels;
}

std::pair<unsigned int, unsigned int> find_cycle(std::string_view moves) {
    Int high_point = 0;
    std::set<Point> occupied;
    std::set<std::array<Int, 9>> seen;
    unsigned int move_idx = 0;
    Point start;
    unsigned int i0 = 0;
    unsigned int i = 0;
    while (true) {
        Rock rock = rocks[i % rocks.size()];
        iterate(moves, rock, move_idx, occupied, high_point);
        std::array<Int, 9> key;
        key[0] = static_cast<Int>(i % rocks.size());
        key[1] = static_cast<Int>(move_idx);
        std::array<Int, 7> levels = bottom_scan(occupied, high_point);
        for (int k = 2; k < 9; ++k) {
            key[k] = levels[k - 2];
        }
        if (key[0] == 0 && seen.count(key)) {
            i0 = i;
            start = Point(key[0], key[1]);
            break;
        }
        seen.insert(key);
        ++i;
    }
    ++i;
    unsigned int len = 1;
    while (true) {
        Rock rock = rocks[i % rocks.size()];
        iterate(moves, rock, move_idx, occupied, high_point);
        Point indices {static_cast<int>(i % rocks.size()), static_cast<int>(move_idx)};
        if (indices == start) {
            break;
        }
        ++len;
        ++i;
    }
    return {i0 - len, len};
}



Int part_two(std::string_view moves) {
    Int N = 1000000000000;
    // Int high_point = 0;
    // std::set<Point> occupied;
    // high_point = run_iterations(100, moves, occupied, high_point);
    // //print(occupied, high_point);
    // return N;
    auto [start, len] = find_cycle(moves);
    std::cout << start << ' ' << len << '\n';
    //unsigned Int start = 17;
    //unsigned Int len = 35;
    Int high_point = 0;
    unsigned int move_idx = 0;
    std::set<Point> occupied;
    Int mile_stone = run_iterations(start, moves, move_idx, occupied, high_point);
    Int diff = high_point - mile_stone;
    high_point = mile_stone;
    mile_stone = run_iterations(len, moves, move_idx, occupied, high_point);
    diff = high_point - mile_stone;
    N -= start + len;

    Int quot = N / len;
    Int rem  = N % len;
    high_point = mile_stone;
    mile_stone = run_iterations(rem, moves, move_idx, occupied, high_point);
    std::cout << rem << '\n';
    return -mile_stone + diff * quot;
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

        Int ans_one = part_one(moves);
        Int ans_two = part_two(moves);

        std::cout << file << ":\n";
        std::cout << "Answer part 1:  " << ans_one << '\n';
        std::cout << "Answer part 2:  " << ans_two << '\n';
    }
    return 0;
}


/*

1514285714288
1514285714288

..#.###
.#####.
....#..
....#..
....#..
....#..
.##.#..
.##.#..
..###..
...#...
..###..
...#...
..####.
.....##
.....##
......#
......#
...####
..###..
...#...
#..####
#...#..
#...#..
#...##.
##..##.
######.
.###...
..#....
.####..
....##.
....##.
....#..
..#.#..
..#.#..
#####..
..###..
...#...
..####.
....#..
....#..
..####.
....###
.....#.
.#####.
.#..#..
.#..#..
.####.#
.####.#
###.###
.#####.
.###...
.###...
.#.#...
.#.#.#.
.######
.#####.
....#..
....#..
....#..
....#..
.##.#..
.##.#..
..###..
....#..
...###.
#...#..
#####..
#.#....
#.#....
####...
..#####
...#.##
..####.
.##....
.##...#
..#...#
..#.###
..#..#.
..#.###
.#####.


*/