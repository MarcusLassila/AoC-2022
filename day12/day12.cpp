#include <algorithm>
#include <array>
#include <cstddef>
#include <fstream>
#include <iostream>
#include <limits>
#include <queue>
#include <string>
#include <vector>

struct Point {
    int x, y;
    Point() : x(0), y(0) {}
    Point(int x, int y) : x(x), y(y) {}

    friend std::ostream& operator<<(std::ostream& os, const Point& p);
};

std::ostream& operator<<(std::ostream& os, const Point& p) {
    os << '(' << p.x << ", " << p.y << ')';
    return os;
}

bool operator==(const Point& p, const Point& q) {
    return p.x == q.x && p.y == q.y;
}

Point operator+(const Point& p, const Point& q) {
    return {p.x + q.x, p.y + q.y};
}

static const std::array<Point, 4> directions {{{-1, 0}, {0, 1}, {1, 0}, {0, -1}}}; 

std::size_t shortest_path(const std::vector<std::string>& grid, const Point& start, const Point& dest) {
    int num_rows = static_cast<int>(grid.size());
    int num_cols = static_cast<int>(grid.front().size());
    std::size_t path_length = std::numeric_limits<std::size_t>::max();  // return max size if no path is found

    const auto in_range = [&grid, num_cols, num_rows](const Point& p) -> bool {
        return 0 <= p.x && p.x < num_rows && 0 <= p.y && p.y < num_cols;
    };

    const auto in_reach = [&grid](const Point& p, const Point& q) -> bool {
        return grid[p.x][p.y] + 1 >= grid[q.x][q.y];
    };

    auto visited = std::vector<std::vector<bool>>(num_rows, std::vector<bool>(num_cols, false));
    std::queue<std::pair<std::size_t, Point>> todo;
    todo.emplace(0, start);

    while (!todo.empty()) {
        const auto& [d, p] = todo.front();
        todo.pop();
        if (p == dest) {
            path_length = d;
            break;
        }
        if (visited[p.x][p.y]) {
            continue;
        }
        visited[p.x][p.y] = true;
        for (const auto& move : directions) {
            Point q = p + move;
            if (in_range(q) && in_reach(p, q)) {
                todo.emplace(d + 1, q);
            }
        }
    }
    return path_length;
}

int main() {
    for (auto file : {"sample.txt", "input.txt"}) {
        std::ifstream input {file};
        if (!input) {
            std::cerr << "Failed to open: " << file << '\n';
            return 1;
        }
        std::string line;
        std::vector<std::string> grid;
        while (std::getline(input, line)) {
            grid.push_back(line);
        }
        Point start;
        Point dest;
        std::vector<Point> low_points;
        for (int i = 0; i < static_cast<int>(grid.size()); ++i) {
            for (int j = 0; j < static_cast<int>(grid.front().size()); ++j) {
                switch (grid[i][j]) {
                    case 'S':
                        grid[i][j] = 'a';
                        start = Point(i, j);
                        low_points.emplace_back(i, j);
                        break;
                    case 'E':
                        grid[i][j] = 'z';
                        dest = Point(i, j);
                        break;
                    case 'a':
                        low_points.emplace_back(i, j);
                        break;
                }
            }
        }
        std::size_t ans_one = shortest_path(grid, start, dest);
        std::vector<std::size_t> path_lengths;
        std::transform(low_points.begin(),
                       low_points.end(),
                       std::back_inserter(path_lengths),
                       [&grid, &dest](const Point& p) -> std::size_t {
                           return shortest_path(grid, p, dest);
                       });
        std::size_t ans_two = *std::min_element(path_lengths.begin(), path_lengths.end());
        std::cout << file << ":\n";
        std::cout << "Answer part 1:  " << ans_one << '\n';
        std::cout << "Answer part 2:  " << ans_two << '\n';
    }
    return 0;
}
