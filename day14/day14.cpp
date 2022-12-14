#include <algorithm>
#include <array>
#include <fstream>
#include <iostream>
#include <iterator>
#include <set>
#include <sstream>
#include <string>
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

int sign(int x) {
    return (x > 0) - (x < 0);
}

class SandDropper {
public:
    SandDropper(const std::set<Point>& obstacles) : m_obstacles(obstacles) {
        m_floor = 2 + std::max_element(obstacles.begin(), obstacles.end(), [](const auto& a, const auto& b) -> bool {
            return a.y < b.y;
        })->y;
    }

    Point move_sand(const Point& point) const;
    unsigned int sand_count() const { return static_cast<unsigned int>(m_filled.size()); }

    void drop_until_void() { drop_until_done(false); };
    void drop_until_full() { drop_until_done(true); }
    void clear_sand() { m_filled.clear(); }

private:
    static const inline std::array<Point, 3> steps {{{0, 1}, {-1, 1}, {1, 1}}};
    static const inline Point drop_point {500, 0};

    std::set<Point> m_obstacles;
    std::set<Point> m_filled;
    int m_floor;

    void drop_until_done(bool has_floor);
};

Point SandDropper::move_sand(const Point& point) const {
    for (const auto& step : steps) {
        auto ret = point + step;
        if (!m_obstacles.count(ret) && !m_filled.count(ret)) {
            return ret;
        }
    }
    return point;
}

void SandDropper::drop_until_done(bool has_floor) {
    int limit = has_floor ? m_floor : m_floor - 2;
    while (true) {
        auto curr = drop_point;
        auto next = move_sand(curr);
        if (has_floor && next == curr) {
            m_filled.insert(curr);
            break;
        }
        while (next != curr && next.y < limit) {
            curr = next;
            next = move_sand(curr);
        }
        if (!has_floor && next != curr) {
            break;
        }
        m_filled.insert(curr);
    }
}

std::set<Point> parse_path(const std::string& line) {
    std::set<Point> ret;
    std::istringstream iss {line};
    int x, y;
    std::string arrow;
    iss >> x >> y >> arrow;
    Point prev {x, y};
    while (iss) {
        iss >> x >> y >> arrow;
        Point curr {x, y};
        Point diff {sign(curr.x - prev.x), sign(curr.y - prev.y)};
        while (prev != curr) {
            ret.insert(prev);
            prev += diff;
        }
    }
    ret.insert(prev);
    return ret;
}

int main() {
    for (auto file : {"sample.txt", "input.txt"}) {
        std::ifstream input {file};
        if (!input) {
            std::cerr << "Failed to open: " << file << '\n';
            return 1;
        }
        std::set<Point> obstacles;
        std::string line;
        while (std::getline(input, line)) {
            std::replace(line.begin(), line.end(), ',', ' ');
            std::set<Point> points = parse_path(line);
            obstacles.insert(points.begin(), points.end());
        }
        std::cout << file << ":\n";
        SandDropper sd {obstacles};

        sd.drop_until_void();
        std::cout << "Answer part 1:  " << sd.sand_count() << '\n';
        sd.clear_sand();

        sd.drop_until_full();
        std::cout << "Answer part 2:  " << sd.sand_count() << '\n';
        sd.clear_sand();
    }
    return 0;
}
