#include <array>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <string_view>

namespace {
    constexpr unsigned int screen_pixels = 240U;
    constexpr unsigned int screen_width  = 40U;
    constexpr unsigned int max_cycles    = 300U;  // Some extra spare cycles.
}

int main() {
    for (auto file : {"sample.txt", "input.txt"}) {
        std::ifstream input {file};
        if (!input) {
            std::cerr << "Failed to open: " << file << '\n';
            return 1;
        }
        std::string line;
        unsigned int cycle = 0;                  // Let 0 be the first cycle due zero based indexing.
        std::array<int, max_cycles> regs = {1};  // regs[i] = register value at cycle i.
        std::string screen(max_cycles, '.');

        const auto draw = [&regs, &screen](int cycle) -> void {
            int pos = cycle % screen_width;
            if (regs[cycle] - 1 <= pos && pos <= regs[cycle] + 1) {
                screen[cycle] = '#';
            }
        };

        while (std::getline(input, line)) {
            draw(cycle);
            ++cycle;
            regs[cycle] = regs[cycle - 1];
            std::istringstream iss {line};
            std::string inst;
            iss >> inst;
            if (inst == "noop") {
                continue;
            } else if (inst == "addx") {
                int x;
                iss >> x;
                draw(cycle);
                ++cycle;
                regs[cycle] = regs[cycle - 1] + x;
            }
            if (cycle >= screen_pixels) {
                break;
            }
        }
        int signal_strength_sum = 0;
        for (unsigned int i = 20; i < screen_pixels; i += 40) {
            signal_strength_sum += i * regs[i - 1];
        }
        std::cout << file << "\n";
        std::cout << "Signal strenght sum: " << signal_strength_sum << '\n';
        std::cout << "Screen:\n";
        std::string_view screen_view {screen};
        for (unsigned int i = 0; i < screen_pixels; i += screen_width) {
            std::cout << screen_view.substr(i, screen_width) << '\n';
        }
    }
    return 0;
}
