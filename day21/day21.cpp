#include <algorithm>
#include <cassert>
#include <cstddef>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <numeric>
#include <optional>
#include <sstream>
#include <stack>
#include <string>
#include <vector>

#define DO_VALIDATION

namespace {
    constexpr std::string_view operators {"+-*/"};
}

long long gcd(long long a, long long b) {
    return b ? gcd(b, a % b) : a;
}

struct Fraction {
    long long num, den;
    Fraction(long long num, long long den = 1LL) {
        assert(den != 0);
        long long g = gcd(num, den);
        this->num = num / g;
        this->den = den / g;
    }
    friend std::ostream& operator<<(std::ostream&, const Fraction&);
};

std::ostream& operator<<(std::ostream& os, const Fraction& fraq) {
    os << fraq.num;
    if (fraq.den != 1) {
        os << '/' << fraq.den;
    }
    return os;
}

Fraction operator+(const Fraction& a, const Fraction& b) {
    return {a.num * b.den + a.den * b.num, a.den * b.den};
}

Fraction operator-(const Fraction& a, const Fraction& b) {
    return {a.num * b.den - a.den * b.num, a.den * b.den};
}

Fraction operator*(const Fraction& a, const Fraction& b) {
    return {a.num * b.num, a.den * b.den};
}

Fraction operator/(const Fraction& a, const Fraction& b) {
    return {a.num * b.den, a.den * b.num};
}

long long evaluate(const std::string& expr) {
    std::stack<long long> stk;
    std::istringstream iss {expr};
    std::string tok;
    while (iss >> tok) {
        if (operators.find(tok) == std::string_view::npos) {
            stk.push(std::stoll(tok));
        } else {
            long long top = stk.top();
            stk.pop();
            switch (tok.front()) {
                case '+': stk.top() += top; break;
                case '-': stk.top() -= top; break;
                case '*': stk.top() *= top; break;
                case '/': stk.top() /= top; break;
            }
        }
    }
    return stk.top();
}

/* Assumes a linear equation in postfix form with exactly one variable x occuring one time only */
long long solve(const std::string& equation) {
    std::size_t idx_eq = equation.find('=');
    auto lhs = std::string(equation.begin(), equation.begin() + idx_eq - 1);
    auto rhs = std::string(equation.begin() + idx_eq + 2, equation.end());
    if (std::count(rhs.begin(), rhs.end(), 'x') == 1) {
        std::swap(lhs, rhs);
    }
    std::vector<std::function<Fraction(Fraction)>> apps;
    std::stack<std::optional<long long>> stk;
    std::istringstream iss {lhs};
    std::string tok;
    while (iss >> tok) {
        if (tok == "x") {
            stk.push(std::optional<long long>{});
        } else if (operators.find(tok) == std::string_view::npos) {
            stk.push(std::stoll(tok));
        } else {
            std::optional<long long> a = stk.top();
            stk.pop();
            std::optional<long long> b = stk.top();
            stk.pop();
            if (a && b) {
                switch (tok.front()) {
                    case '+': stk.push(b.value() + a.value()); break;
                    case '-': stk.push(b.value() - a.value()); break;
                    case '*': stk.push(b.value() * a.value()); break;
                    case '/': stk.push(b.value() / a.value()); break;
                    default : throw std::runtime_error("Invalid operator!");
                }
            } else if (!b) {
                stk.push(b);
                apps.push_back([tok, a](Fraction z) -> Fraction {
                    switch (tok.front()) {
                        case '+': return z - a.value();
                        case '-': return z + a.value();
                        case '*': return z / a.value();
                        case '/': return z * a.value();
                        default : throw std::runtime_error("Invalid operator!");
                    }
                });
            } else {
                stk.push(a);
                apps.push_back([tok, b](Fraction z) -> Fraction {
                    switch (tok.front()) {
                        case '+': return z - b.value();
                        case '-': return b.value() - z;
                        case '*': return z / b.value();
                        case '/': return b.value() / z;
                        default : throw std::runtime_error("Invalid operator!");
                    }
                });
            }
        }
    }
    Fraction solution = std::accumulate(apps.rbegin(),
                                        apps.rend(),
                                        Fraction(evaluate(rhs)),
                                        [](const Fraction& acc, const auto& func) -> Fraction {
                                            return func(acc);
                                        });
    assert(solution.den == 1);
    return solution.num;
}

int main() {
    for (auto file : {"sample.txt", "input.txt"}) {
        std::ifstream input {file};
        if (!input) {
            std::cerr << "Failed to open: " << file << '\n';
            return 1;
        }
        std::map<std::string, std::function<long long(void)>> table;
        std::map<std::string, std::function<std::string(void)>> postfix_table;
        postfix_table["humn"] = [](){ return "x"; };
        std::string line;
        while (std::getline(input, line)) {
            std::replace(line.begin(), line.end(), ':', ' ');
            std::istringstream iss {line};
            std::string key, lhs, op, rhs;
            iss >> key >> lhs >> op >> rhs;

            table[key] = [&table, lhs, op, rhs]() -> long long {
                if (op.empty()) {
                    return std::stoll(lhs);
                }
                switch (op.front()) {
                    case '+': return table[lhs]() + table[rhs]();
                    case '-': return table[lhs]() - table[rhs]();
                    case '*': return table[lhs]() * table[rhs]();
                    case '/': return table[lhs]() / table[rhs]();
                    default : throw std::runtime_error("Invalid operator!");
                }
            };

            if (key == "humn") {
                continue;
            }
            if (key == "root") {
                op = "=";
            }
            postfix_table[key] = [&postfix_table, lhs, op, rhs]() -> std::string {
                if (op.empty()) {
                    return lhs;
                }
                if (operators.find(op) != std::string_view::npos) {
                    return postfix_table[lhs]() + " " + postfix_table[rhs]() + " " + op;
                }
                if (op == "=") {
                    return postfix_table[lhs]() + " = " + postfix_table[rhs]();
                }
                throw std::runtime_error("Invalid operator!");
            };
        }
        std::cout << file << ":\n";
        std::cout << "Answer part 1:  " << table["root"]() << '\n';

        std::string equation = postfix_table["root"]();
        long long solution = solve(equation);

#ifdef DO_VALIDATION

        table.clear();
        input.clear();
        input.seekg(0);
        while (std::getline(input, line)) {
            std::replace(line.begin(), line.end(), ':', ' ');
            std::istringstream iss {line};
            std::string key, lhs, op, rhs;
            iss >> key >> lhs >> op >> rhs;
            if (key == "humn") {
                lhs = std::to_string(solution);
            }
            if (key == "root") {
                op = "-";
            }
            table[key] = [&table, lhs, op, rhs]() -> long long {
                if (op.empty()) {
                    return std::stoll(lhs);
                }
                switch (op.front()) {
                    case '+': return table[lhs]() + table[rhs]();
                    case '-': return table[lhs]() - table[rhs]();
                    case '*': return table[lhs]() * table[rhs]();
                    case '/': return table[lhs]() / table[rhs]();
                    default : throw std::runtime_error("Invalid operator!");
                }
            };
        }
        if (table["root"]() == 0)
            std::cout << "Answer part 2:  " << solution << '\n';
        else
            std::cout << "Validation failed, no solution found" << '\n';
#else
        std::cout << "Answer part 2:  " << solution << '\n';
#endif
    }
    return 0;
}
