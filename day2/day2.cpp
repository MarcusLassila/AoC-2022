#include <bits/stdc++.h>

int main() {
  std::ifstream input {"input.txt"};
  if (!input) {
    std::cerr << "Failed to open input file\n";
    return 1;
  }
  std::string line;
  while (std::getline(input, line)) {
    std::cout << line << '\n';
  }
  return 0;
}
