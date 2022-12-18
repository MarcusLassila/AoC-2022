#include <algorithm>
#include <array>
#include <fstream>
#include <iostream>
#include <optional>
#include <queue>
#include <sstream>
#include <string>
#include <tuple>
#include <type_traits>
#include <vector>

struct Point {
    int x, y, z;
    Point() : x(0), y(0), z(0) {}
    Point(int x, int y, int z) : x(x), y(y), z(z) {}

    friend std::ostream& operator<<(std::ostream& os, const Point& p);
};

std::ostream& operator<<(std::ostream& os, const Point& p) {
    os << '(' << p.x << ',' << p.y << ',' << p.z << ')';
    return os;
}

bool operator==(const Point& p, const Point& q) {
    return p.x == q.x && p.y == q.y && p.z == q.z;
}

bool operator!=(const Point& p, const Point& q) {
    return !(p == q);
}

bool operator<(const Point& p, const Point& q) {
    if (p.x < q.x) {
        return true;
    }
    if (p.x > q.x) {
        return false;
    }
    return p.y < q.y || (p.y == q.y && p.z < q.z);
}

template<std::size_t I>
std::tuple_element_t<I, Point>& get(const Point& p) {
    if constexpr (I == 0) return p.x;
    if constexpr (I == 1) return p.y;
    if constexpr (I == 2) return p.z;
}

class Grid_3D {
public:
    using Grid = std::vector<std::vector<std::vector<int>>>;

    Grid_3D(const Grid& grid)
        : m_grid(grid)
        , m_dim(std::make_tuple(static_cast<int>(grid.size()),
                                static_cast<int>(grid.front().size()),
                                static_cast<int>(grid.front().front().size())))
        {}

    bool on_grid(const Point&) const;
    bool occupied(const Point&) const;

    unsigned int exterior_surface_area();
    unsigned int surface_area();

private:
    static const inline std::array<Point, 6> cardinal_directions {{Point{0, 0, 1}, Point{0, 0, -1},
                                                                   Point{0, 1, 0}, Point{0, -1, 0},
                                                                   Point{1, 0, 0}, Point{-1, 0, 0}}};
    Grid m_grid;
    std::tuple<int, int, int> m_dim;
    std::optional<unsigned int> m_surface_area;
};

bool Grid_3D::on_grid(const Point& p) const {
    const auto& [M, N, K] = m_dim;
    return 0 <= p.x && p.x < M && 0 <= p.y && p.y < N && 0 <= p.z && p.z < K;
}

bool Grid_3D::occupied(const Point& p) const {
    return on_grid(p) && m_grid[p.x][p.y][p.z];
}

unsigned int Grid_3D::surface_area() {
    if (m_surface_area) {
        return m_surface_area.value();
    }
    const auto& [M, N, K] = m_dim;
    unsigned int area = 0;
    for (int x = 0; x < M; ++x) {
        for (int y = 0; y < N; ++y) {
            for (int z = 0; z < K; ++z) {
                if (!m_grid[x][y][z]) {
                    continue;
                }
                for (const auto& [dx, dy, dz] : cardinal_directions) {
                    if (!occupied(Point{x + dx, y + dy, z + dz})) {
                        ++area;
                    }
                }
            }
        }
    }
    m_surface_area = area;
    return area;
}

unsigned int Grid_3D::exterior_surface_area() {
    const auto& [M, N, K] = m_dim;
    unsigned int interior_area = 0;
    auto processed = std::vector(M, std::vector(N, std::vector<bool>(K, false)));

    const auto connected_surface_area = [this, &processed](const Point& start) -> unsigned int {
        unsigned int area = 0;
        std::queue<Point> todo;
        todo.push(start);
        bool is_exterior = false;
        while (!todo.empty()) {
            auto&& [x, y, z] = todo.front();
            todo.pop();
            if (processed[x][y][z]) {
                continue;
            }
            processed[x][y][z] = true;
            for (const auto& [dx, dy, dz]: cardinal_directions) {
                Point adj {x + dx, y + dy, z + dz};
                if (!on_grid(adj)) {
                    is_exterior = true;
                    continue;
                }
                if (occupied(adj))
                    ++area;
                else
                    todo.push(adj);
            }
        }
        return is_exterior ? 0 : area;
    };

    for (int x = 0; x < M; ++x) {
        for (int y = 0; y < N; ++y) {
            for (int z = 0; z < K; ++z) {
                if (m_grid[x][y][z] || processed[x][y][z]) {
                    continue;
                }
                interior_area += connected_surface_area(Point{x, y, z});;
            }
        }
    }
    return surface_area() - interior_area;
}

int main() {
    for (auto file : {"sample.txt", "input.txt"}) {
        std::ifstream input {file};
        if (!input) {
            std::cerr << "Failed to open: " << file << '\n';
            return 1;
        }
        std::string line;
        std::vector<int> xs, ys, zs;
        while (std::getline(input, line)) {
            std::replace(line.begin(), line.end(), ',', ' ');
            std::istringstream iss {line};
            int x, y, z;
            iss >> x >> y >> z;
            xs.push_back(x);
            ys.push_back(y);
            zs.push_back(z);
        }
        const auto& [x_min, x_max] = std::minmax_element(xs.begin(), xs.end());
        const auto& [y_min, y_max] = std::minmax_element(ys.begin(), ys.end());
        const auto& [z_min, z_max] = std::minmax_element(zs.begin(), zs.end());

        auto grid = std::vector(*x_max - *x_min + 1, std::vector(*y_max - *y_min + 1, std::vector(*z_max - *z_min + 1, 0)));
        for (std::size_t i = 0; i < xs.size(); ++i) {
            grid[xs[i] - *x_min][ys[i] - *y_min][zs[i] - *z_min] = 1;
        }

        Grid_3D g_3D {grid};
        std::cout << file << ":\n";
        std::cout << "Answer part 1:  " << g_3D.surface_area() << '\n';
        std::cout << "Answer part 2:  " << g_3D.exterior_surface_area() << '\n';
    }
    return 0;
}
