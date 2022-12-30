#include <fstream>
#include <iostream>
#include <stack>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

int part_one(const std::vector<std::string>& grid) {
    int M = static_cast<int>(grid.size());
    int N = static_cast<int>(grid.front().size());
    int top = 0;
    std::unordered_set<int> seen;

    const auto flatten = [N](int row, int col) -> int {
        return row * N + col;
    };

    const auto process = [&](int row, int col) -> void {
        if (grid[row][col] > top) {
            seen.insert(flatten(row, col));
            top = grid[row][col];
        }
    };

    for (int i = 0; i < M; ++i) {
        seen.insert(flatten(i, 0));
        top = grid[i][0];
        for (int j = 1; j < N; ++j) {
            process(i, j);
        }
        seen.insert(flatten(i, N - 1));
        top = grid[i][N - 1];
        for (int j = N - 2; j >= 0; --j) {
            process(i, j);
        }
    }

    for (int j = 0; j < N; ++j) {
        seen.insert(flatten(0, j));
        top = grid[0][j];
        for (int i = 1; i < M; ++i) {
            process(i, j);
        }
        seen.insert(flatten(M - 1, j));
        top = grid[M - 1][j];
        for (int i = M - 2; i >= 0; --i) {
            process(i, j);
        }
    }

    return static_cast<int>(seen.size());
}

int part_two(const std::vector<std::string>& grid) {
    int M = static_cast<int>(grid.size());
    int N = static_cast<int>(grid.front().size());

    const auto evaluate = [&](const auto& coord) -> char {
        const auto& [row, col] = coord;
        return grid[row][col];
    };

    const auto pop_while_smaller = [&evaluate](char val, auto& stk) -> void {
        while (!stk.empty() && evaluate(stk.top()) < val) {
            stk.pop();
        }
    };

    const auto length_down = [M, N](int row, auto& stk) -> int {
        return stk.empty() ? row : row - stk.top().first;
    };

    const auto length_up = [M, N](int row, auto& stk) -> int {
        return stk.empty() ? M - 1 - row : stk.top().first;
    };

    const auto length_left = [N](int col, auto& stk) -> int {
        return stk.empty() ? col : col - stk.top().second;
    };

    const auto length_right = [N](int col, auto& stk) -> int {
        return stk.empty() ? N - 1 - col : stk.top().second - col;
    };

    const auto first_not_smaller = [&](int row, int col, auto& stk, auto&& length_callback) -> int {
        pop_while_smaller(grid[row][col], stk);
        int ret = length_callback(stk);
        stk.emplace(row, col);
        return ret;
    };

    std::vector<std::vector<int>> scenic_table(M, std::vector<int>(N, 1));
    
    for (int i = 0; i < M; ++i) {
        std::stack<std::pair<int, int>> stk;
        stk.emplace(i, 0);
        for (int j = 1; j < N; ++j) {
            scenic_table[i][j] *= first_not_smaller(i, j, stk, [&](auto& stk){ return length_left(j, stk); });
        }
        stk = {};
        stk.emplace(i, N - 1);
        for (int j = N - 2; j >= 0; --j) {
            scenic_table[i][j] *= first_not_smaller(i, j, stk, [&](auto& stk){ return length_right(j, stk); });
        }
    }

    for (int j = 0; j < N; ++j) {
        std::stack<std::pair<int, int>> stk;
        stk.emplace(0, j);
        for (int i = 1; i < M; ++i) {
            scenic_table[i][j] *= first_not_smaller(i, j, stk, [&](auto& stk){ return length_down(i, stk); });
        }
        stk = {};
        stk.emplace(M - 1, j);
        for (int i = M - 2; i >= 0; --i) {
            scenic_table[i][j] *= first_not_smaller(i, j, stk, [&](auto& stk){ return length_up(i, stk); });
        }
    }

    int max_scenic = 0;

    for (int i = 1; i + 1 < M; ++i) {
        for (int j = 1; j + 1 < N; ++j) {
            max_scenic = std::max(scenic_table[i][j], max_scenic);
        }
    }
    return max_scenic;
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
        std::cout << file << ":\n";
        std::cout << "Answer part 1:  " << part_one(grid) << '\n';
        std::cout << "Answer part 2:  " << part_two(grid) << '\n';
    }
    return 0;
}
