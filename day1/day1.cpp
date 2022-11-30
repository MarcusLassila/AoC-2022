#include <bits/stdc++.h>

int solve() {
  return 0;
}

int main() {
  std::ifstream input {"input.txt"};
  if (!input) {
    std::cerr << "Failed to open input file\n";
    return 1;
  }
  int num = 0;
  while (input >> num) {
    std::cout << num << '\n';
  }
  std::cout << "Answer: " << solve() << '\n'; 
  return 0;
}
