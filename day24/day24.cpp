#include <algorithm>
#include <cstddef>
#include <fstream>
#include <iostream>
#include <limits>
#include <map>
#include <queue>
#include <sstream>
#include <string>
#include <unordered_set>
#include <utility>
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

class BlizzardBasin {
public:
    using Grid     = std::vector<std::string>;
    using TimeGrid = std::vector<Grid>;
    BlizzardBasin(const Grid& initial_layer);

    unsigned int shortest_path_time(unsigned int initial_time, const Point& start, const Point& end) const;

private:
    TimeGrid m_grid;
    std::pair<int, int> m_dimensions;
    static const inline std::map<char, Point> directions = {{'>', { 0,  1}},
                                                            {'v', { 1,  0}},
                                                            {'<', { 0, -1}},
                                                            {'^', {-1,  0}}};
};

BlizzardBasin::BlizzardBasin(const Grid& initial_layer)
    : m_dimensions{std::make_pair(static_cast<int>(initial_layer.size()),
                                  static_cast<int>(initial_layer.front().size()))}
{
    using State = std::vector<std::vector<std::string>>;
    State state;
    for (const auto& row : initial_layer) {
        std::vector<std::string> state_row;
        for (char tok : row) {
            if (tok == '.')
                state_row.push_back(""s);
            else
                state_row.push_back(std::string(1, tok));
        }
        state.push_back(state_row);
    }

    const auto tick = [this](const State& state) -> State {
        const auto& [M, N] = m_dimensions;
        State new_state = std::vector(M, std::vector(N, ""s));

        for (int x = 0; x < M; ++x) {
            new_state[x][0] = "#"s;
            new_state[x][N - 1] = "#"s;
        }
        for (int y = 0; y < N; ++y) {
            new_state[0][y] = "#"s;
            new_state[M - 1][y] = "#"s;
        }
        new_state[0][1] = ""s;
        new_state[M - 1][N - 2] = ""s;

        for (int x = 1; x + 1 < M; ++x) {
            for (int y = 1; y + 1 < N; ++y) {
                for (char tok : state[x][y]) {
                    auto it = directions.find(tok);
                    if (it == directions.end()) {
                        continue;
                    }
                    auto [u, v] = Point{x, y} + it->second;
                    if (u == 0 && tok == '^') u = M - 2;
                    if (v == 0 && tok == '<') v = N - 2;
                    if (u == M - 1 && tok == 'v') u = 1;
                    if (v == N - 1 && tok == '>') v = 1;
                    new_state[u][v].push_back(tok);
                }
            }
        }
        return new_state;
    };

    const auto to_grid = [](const State& state) -> Grid {
        Grid layer;
        for (const auto& state_row : state) {
            std::string row;
            std::transform(state_row.cbegin(), state_row.cend(), std::back_inserter(row), [](const auto& tokens) -> char {
                if (tokens.empty()) {
                    return '.';
                }
                std::size_t winds = std::count_if(tokens.begin(), tokens.end(), [](char tok) {
                    return directions.find(tok) != directions.end();
                });
                if (winds > 1) {
                    return static_cast<char>(winds) + '0';
                }
                return tokens.front();
            });
            layer.push_back(row);
        }
        return layer;
    };

    const auto flatten = [](const Grid& layer) -> std::string {
        std::stringstream ss;
        for (const auto& row : layer) {
            ss << row;
        }
        return ss.str();
    };

    std::unordered_set<std::string> seen;
    seen.insert(flatten(initial_layer));
    m_grid.push_back(initial_layer);
    while (true) {
        state = tick(state);
        auto layer = to_grid(state);
        auto key = flatten(layer);
        if (seen.count(key)) {
            break;
        }
        m_grid.push_back(layer);
        seen.insert(flatten(layer));
    }
}

unsigned int BlizzardBasin::shortest_path_time(unsigned int initial_time,
                                               const Point& start,
                                               const Point& end) const
{
    unsigned int shortest_time = std::numeric_limits<unsigned int>::max();
    unsigned int time_cycle    = static_cast<unsigned int>(m_grid.size());

    const auto& [M, N] = m_dimensions;

    auto seen = std::vector(time_cycle, std::vector(M, std::vector(N, false)));
    std::queue<std::pair<unsigned int, Point>> todo;
    todo.emplace(std::make_pair(initial_time, start));

    while (!todo.empty()) {
        const auto& [t, p] = todo.front();
        const auto& [x, y] = p;
        todo.pop();
        if (seen[t % time_cycle][x][y]) {
            continue;
        }
        seen[t % time_cycle][x][y] = true;
        if (p == end) {
            shortest_time = t;
            break;
        }
        unsigned int tt = t + 1;
        if (m_grid[tt % time_cycle][x][y] == '.') {
            todo.emplace(std::make_pair(tt, p));
        }
        for (const auto& [_, dp]: directions) {
            const auto& [u, v] = p + dp;
            if (0 <= u && u < M && 0 <= v && v < N && m_grid[tt % time_cycle][u][v] == '.') {
                todo.emplace(std::make_pair(tt, Point{u, v}));
            }
        }
    }
    return shortest_time;
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
        while (std::getline(input, line)) {
            grid.push_back(line);
        }

        int M = static_cast<int>(grid.size());
        int N = static_cast<int>(grid.front().size());
        Point start {0, 1};
        Point end   {M - 1, N - 2};
        BlizzardBasin blizbas {grid};
        unsigned int t1 = blizbas.shortest_path_time(0, start, end);
        unsigned int t2 = blizbas.shortest_path_time(t1, end, start);
        unsigned int t3 = blizbas.shortest_path_time(t2, start, end);
        std::cout << file << ":\n";
        std::cout << "Answer part 1:  " << t1 << '\n';
        std::cout << "Answer part 2:  " << t3 << '\n';
    }
    return 0;
}
