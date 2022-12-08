#include <fstream>
#include <iostream>
#include <string>
#include <unordered_set>
#include <vector>

int part_one(const std::vector<std::string>& grid) {
    int M = grid.size();
    int N = grid.front().size();
    std::unordered_set<int> seen;
    const auto flatten = [N](int row, int col) -> int {
        return row * N + col;
    };

    for (int i = 0; i < M; ++i) {
        seen.insert(flatten(i, 0));
        int top = grid[i][0];
        for (int j = 1; j < N; ++j) {
            if (grid[i][j] > top) {
                seen.insert(flatten(i, j));
                top = grid[i][j];
            }
        }
        seen.insert(flatten(i, N - 1));
        top = grid[i][N - 1];
        for (int j = N - 2; j >= 0; --j) {
            if (grid[i][j] > top) {
                seen.insert(flatten(i, j));
                top = grid[i][j];
            }
        }
    }

    for (int j = 0; j < N; ++j) {
        seen.insert(flatten(0, j));
        int top = grid[0][j];
        for (int i = 1; i < M; ++i) {
            if (grid[i][j] > top) {
                seen.insert(flatten(i, j));
                top = grid[i][j];
            }
        }
        seen.insert(flatten(M - 1, j));
        top = grid[M - 1][j];
        for (int i = M - 2; i >= 0; --i) {
            if (grid[i][j] > top) {
                seen.insert(flatten(i, j));
                top = grid[i][j];
            }
        }
    }

    return seen.size();
}

int part_two(const std::vector<std::string>& grid) {
    int M = grid.size();
    int N = grid.front().size();
    
    const auto scenic_score = [&](int row, int col) -> int {
        int up     = 0;
        int down   = 0;
        int left   = 0;
        int right  = 0;

        for (int k = row - 1; k >= 0; --k) {
            ++up;
            if (grid[k][col] >= grid[row][col]) {
                break;
            }
        }
        for (int k = row + 1; k < M; ++k) {
            ++down;
            if (grid[k][col] >= grid[row][col]) {
                break;
            }
        }
        for (int k = col - 1; k >= 0; --k) {
            ++left;
            if (grid[row][k] >= grid[row][col]) {
                break;
            }
        }
        for (int k = col + 1; k < N; ++k) {
            ++right;
            if (grid[row][k] >= grid[row][col]) {
                break;
            }
        }
        return up * down * left * right;
    };

    int max_scenic = 0;

    for (int i = 1; i + 1< M; ++i) {
        for (int j = 1; j + 1 < N; ++j) {
            max_scenic = std::max(scenic_score(i, j), max_scenic);
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
