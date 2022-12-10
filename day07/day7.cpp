#include <algorithm>
#include <fstream>
#include <iostream>
#include <limits>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

namespace {
    constexpr std::size_t size_limit    = 100000U;
    constexpr std::size_t total_space   = 70000000U;
    constexpr std::size_t needed_space  = 30000000U;
}

class Directory {
public:
    static constexpr inline std::size_t NA_SIZE = std::numeric_limits<std::size_t>::max();

    Directory(const std::string& name, Directory* parent = nullptr)
        : m_name(name)
        , m_parent(parent)
        {}

    void set_size(std::size_t new_size);
    void add_subdirectory(const std::string& name);
    void add_subdirectory(std::unique_ptr<Directory> subdir_ptr);

    std::string name() const { return m_name; }
    std::size_t size() const { return m_size; }
    Directory* parent() const { return m_parent; }

    const std::vector<std::unique_ptr<Directory>>& subdirectories() const { return m_subdirs; }
    std::vector<std::unique_ptr<Directory>>& subdirectories() { return m_subdirs; }

private:
    std::string m_name;
    std::size_t m_size = 0;
    Directory* m_parent;
    std::vector<std::unique_ptr<Directory>> m_subdirs;
};

void Directory::add_subdirectory(const std::string& name) {
    m_subdirs.emplace_back(std::make_unique<Directory>(name, this));
}

void Directory::add_subdirectory(std::unique_ptr<Directory> subdir_ptr) {
    m_subdirs.push_back(std::move(subdir_ptr));
}

void Directory::set_size(std::size_t new_size) {
    std::size_t old_size = m_size;
    m_size = new_size;
    Directory* ptr = m_parent;
    while (ptr) {
        ptr->m_size -= old_size;
        ptr->m_size += new_size;
        ptr = ptr->m_parent;
    }
}

std::size_t sum_if_below(const Directory* root, std::size_t limit) {
    std::size_t sum = root->size() < limit ? root->size() : 0;
    for (const auto& ptr : root->subdirectories()) {
        sum += sum_if_below(ptr.get(), limit);
    }
    return sum;
}

/* smallest size of directory tree of root that is >= n */
std::size_t least_larger(Directory* root, std::size_t n) {
    if (root->size() < n) {
        return Directory::NA_SIZE;
    }
    std::size_t ret = root->size();
    for (const auto& ptr : root->subdirectories()) {
        ret = std::min(least_larger(ptr.get(), n), ret);
    }
    return ret;
}

std::size_t size_to_remove(Directory* root) {
    std::size_t unused_space  = total_space - root->size();
    std::size_t space_to_free = needed_space - unused_space;
    return least_larger(root, space_to_free);
}

/* Assumes well-behaved imput */
std::unique_ptr<Directory> parse_terminal_output(const std::string& filename) {
    std::ifstream input {filename};
    if (!input) {
        std::cerr << "Failed to open: " << filename << '\n';
        return nullptr;
    }
    auto root = std::make_unique<Directory>("/");
    auto curr = root.get();
    std::string line;
    std::getline(input, line);  // Skip first line
    while (std::getline(input, line)) {
        std::istringstream iss {line};
        std::string word;
        iss >> word;
        if (word == "$") {
            iss >> word;
            if (word == "ls") {
                continue;
            }
            iss >> word;
            if (word == "..") {
                curr = curr->parent();
            } else {
                curr = std::find_if(curr->subdirectories().cbegin(),
                                    curr->subdirectories().cend(),
                                    [&word](const auto& ptr) -> bool {
                                        return ptr->name() == word;
                                    })->get();
            }
        } else if (word == "dir") {
            iss >> word;
            curr->add_subdirectory(word);
        } else {  // File begining with size
            curr->set_size(curr->size() + std::stoul(word));
        }
    }
    return root;
}

int main() {
    for (auto filename : {"sample.txt", "input.txt"}) {
        std::unique_ptr<Directory> root = parse_terminal_output(filename);
        if (!root) {
            return 1;
        }
        std::cout << filename << ":\n";
        std::cout << "Answer part 1:  " << sum_if_below(root.get(), size_limit) << '\n';
        std::cout << "Answer part 2:  " << size_to_remove(root.get()) << '\n';
    }
    return 0;
}
