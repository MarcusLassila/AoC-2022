#include <fstream>
#include <iostream>
#include <set>
#include <string>
#include <string_view>

constexpr unsigned int packet_marker_length = 4;
constexpr unsigned int message_marker_length = 14;

int find_marker(const std::string_view& signal, unsigned int len) {
    for (unsigned int i = 0; i < signal.length() - len + 1; ++i) {
        auto window  = signal.substr(i, len);
        auto uniques = std::set<int> {window.begin(), window.end()};
        if (uniques.size() == len) {
            return i + len;
        }
    }
    return -1;
}

int main() {
    for (auto file : {"sample.txt", "input.txt"}) {
        std::ifstream input {file};
        if (!input) {
            std::cerr << "Failed to open: " << file << '\n';
            return 1;
        }
        std::string signal;
        std::getline(input, signal);
        std::cout << file << ":\n";
        std::cout << "Answer part 1:  " << find_marker(signal, packet_marker_length) << '\n';
        std::cout << "Answer part 2:  " << find_marker(signal, message_marker_length) << '\n';
    }
    return 0;
}
