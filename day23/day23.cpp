#include <algorithm>
#include <array>
#include <cstddef>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <vector>

using namespace std::string_literals;

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
        std::size_t h1 = std::hash<int>{}(p.x);
        std::size_t h2 = std::hash<int>{}(p.y);
        return h1 ^ (h2 << 1);
    }
};

template<std::size_t I>
std::tuple_element_t<I, Point>& get(const Point& p) {
    if constexpr (I == 0) return p.x;
    if constexpr (I == 1) return p.y;
}

class ElvAutomata {
public:
    ElvAutomata(const std::unordered_set<Point>& elves_pos)
        : m_elves(elves_pos)
        , m_considerations({{"N"s, "NE"s, "NW"s},
                            {"S"s, "SE"s, "SW"s},
                            {"W"s, "NW"s, "SW"s},
                            {"E"s, "NE"s, "SE"s}})
        {}

    void evolve(unsigned int steps);
    unsigned int evolve_until_done();

    const std::unordered_set<Point>& elves_pos() const { return m_elves; }
    std::tuple<int, int, int, int> cropped_dimensions() const;
    friend std::ostream& operator<<(std::ostream&, const ElvAutomata&);

private:
    std::unordered_set<Point> m_elves;
    std::vector<std::array<std::string, 3>> m_considerations;

    static const inline std::map<std::string, Point> directions = {{"N"s, Point(-1,  0)}, {"NE"s, Point(-1,  1)},
                                                                   {"E"s, Point( 0,  1)}, {"SE"s, Point( 1,  1)},
                                                                   {"S"s, Point( 1,  0)}, {"SW"s, Point( 1, -1)},
                                                                   {"W"s, Point( 0, -1)}, {"NW"s, Point(-1, -1)}};

    bool evolve_once();
};

void ElvAutomata::evolve(unsigned int steps) {
    while (steps--) evolve_once();
}

unsigned int ElvAutomata::evolve_until_done() {
    unsigned int steps = 1;
    while (evolve_once()) ++steps;
    return steps;
}

bool ElvAutomata::evolve_once() {
    std::map<Point, std::vector<Point>> proposals;
    std::size_t alone_count = 0;
    for (const auto& elv : m_elves) {
        bool is_alone = std::all_of(directions.cbegin(),
                                    directions.cend(),
                                    [this, &elv](const auto& item) -> bool {
                                        return m_elves.count(elv + item.second) == 0;
                                    });
        if (is_alone) {
            ++alone_count;
            continue;
        }
        auto considerations = m_considerations;
        for (const auto& dirs : considerations) {
            bool is_blocked = std::any_of(dirs.begin(),
                                          dirs.end(),
                                          [this, &elv](const auto& step) -> bool {
                                            return m_elves.count(elv + directions.at(step));
                                          });
            if (is_blocked) {
                continue;
            }
            Point prop = elv + directions.at(dirs.front());
            proposals[prop].push_back(elv);
            break;
        }
    }
    if (alone_count == m_elves.size()) {
        return false;
    }
    for (const auto& [prop, elves] : proposals) {
        if (elves.size() == 1) {
            m_elves.insert(prop);
            m_elves.erase(elves.front());
        }
    }
    auto front = m_considerations.front();
    m_considerations.erase(m_considerations.begin());
    m_considerations.insert(m_considerations.end(), front);
    return true;
}

std::tuple<int, int, int, int> ElvAutomata::cropped_dimensions() const {
    const auto& [x_min, x_max] = std::minmax_element(m_elves.cbegin(),
                                                     m_elves.cend(),
                                                     [](const auto& a, const auto& b) -> bool {
                                                        return a.x < b.x;
                                                     });
    const auto& [y_min, y_max] = std::minmax_element(m_elves.cbegin(),
                                                     m_elves.cend(),
                                                     [](const auto& a, const auto& b) -> bool {
                                                        return a.y < b.y;
                                                     });
    return {x_min->x, x_max->x + 1, y_min->y, y_max->y + 1};
}

std::ostream& operator<<(std::ostream& os, const ElvAutomata& elv_automata) {
    const auto& [m, M, n, N] = elv_automata.cropped_dimensions();
    for (int i = m; i < M; ++i) {
        for (int j = n; j < N; ++j) {
            char tok = elv_automata.elves_pos().count({i, j}) ? '#' : '.';
            os << tok;
        }
        os << '\n';
    }
    return os;
}

unsigned int count_unoccupied(const ElvAutomata& ea) {
    unsigned int ret = 0;
    const auto& [m, M, n, N] = ea.cropped_dimensions();
    for (int i = m; i < M; ++i) {
        for (int j = n; j < N; ++j) {
            if (!ea.elves_pos().count({i, j})) {
                ++ret;
            }
        }
    }
    return ret;
}

int main() {
    for (auto file : {"sample.txt", "input.txt"}) {
        std::ifstream input {file};
        if (!input) {
            std::cerr << "Failed to open: " << file << '\n';
            return 1;
        }
        int row = 0;
        std::unordered_set<Point> elves_pos;
        std::string line;
        while (std::getline(input, line)) {
            int col = 0;
            for (auto tok : line) {
                if (tok == '#') {
                    elves_pos.insert({row, col});
                }
                ++col;
            }
            ++row;
        }
        const unsigned int first_iterations = 10;
        ElvAutomata elv_automata {elves_pos};
        elv_automata.evolve(first_iterations);
        unsigned int ans_one = count_unoccupied(elv_automata);
        unsigned int ans_two = elv_automata.evolve_until_done() + first_iterations;
        std::cout << file << ":\n";
        std::cout << "Answer part 1:  " << ans_one << '\n';
        std::cout << "Answer part 2:  " << ans_two << '\n';
    }
    return 0;
}
