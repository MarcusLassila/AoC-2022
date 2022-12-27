#include <algorithm>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

struct Blueprint {
    int id;
    int ore;
    int clay;
    int obsidian_ore;
    int obsidian_clay;
    int geode_ore;
    int geode_obsidian;
    Blueprint(int id, int ore, int clay, int obsidian_ore, int obsidian_clay, int geode_ore, int geode_obsidian)
        : id(id)
        , ore(ore)
        , clay(clay)
        , obsidian_ore(obsidian_ore)
        , obsidian_clay(obsidian_clay)
        , geode_ore(geode_ore)
        , geode_obsidian(geode_obsidian)
        {}
};

struct Stuff {
    int ore;
    int clay;
    int obsidian;
    int geode;
    Stuff(int ore, int clay, int obsidian, int geode)
        : ore(ore)
        , clay(clay)
        , obsidian(obsidian)
        , geode(geode)
        {}
    Stuff() : Stuff(0, 0, 0, 0) {}
    friend std::ostream& operator<<(std::ostream&, const Stuff&);
};

std::ostream& operator<<(std::ostream& os, const Stuff& minerals) {
    os << "Ore: " << minerals.ore;
    os << " Clay: " << minerals.clay;
    os << " Obsidian: " << minerals.obsidian;
    os << " Geode: " << minerals.geode;
    return os;
}

struct Minerals : Stuff {
    Minerals() : Stuff() {}
    Minerals(int ore, int clay, int obsidian, int geode) : Stuff(ore, clay, obsidian, geode) {}
};

struct Robots : Stuff {
    Robots() : Stuff() {}
    Robots(int ore, int clay, int obsidian, int geode) : Stuff(ore, clay, obsidian, geode) {}
};

bool operator==(const Stuff& a, const Stuff& b) {
    return a.ore == b.ore && a.clay == b.clay && a.obsidian == b.obsidian && a.geode == b.geode;
}

bool operator!=(const Stuff& a, const Stuff& b) {
    return !(a == b);
}

bool operator<(const Stuff& a, const Stuff& b) {
    if (a.geode < b.geode) {
        return true;
    }
    if (a.geode > b.geode) {
        return false;
    }
    if (a.obsidian < b.obsidian) {
        return true;
    }
    if (a.obsidian > b.obsidian) {
        return false;
    }
    if (a.clay < b.clay) {
        return true;
    }
    if (a.clay > b.clay) {
        return false;
    }
    return a.ore < b.ore;
}

struct State {
    Minerals minerals;
    Robots robots;
    int time = 0;
    State(const Minerals& minerals, const Robots& robots, int time)
        : minerals(minerals)
        , robots(robots)
        , time(time)
        {}

    void tick();
    void untick();

    friend std::ostream& operator<<(std::ostream&, const State&);
};

void State::tick() {
    minerals.ore      += robots.ore;
    minerals.clay     += robots.clay;
    minerals.obsidian += robots.obsidian;
    minerals.geode    += robots.geode;
    ++time;
}

void State::untick() {
    minerals.ore      -= robots.ore;
    minerals.clay     -= robots.clay;
    minerals.obsidian -= robots.obsidian;
    minerals.geode    -= robots.geode;
    --time;
}

std::ostream& operator<<(std::ostream& os, const State& state) {
    os << "Minerals:  " << state.minerals << '\n';
    os << "Robots:  " << state.robots << '\n';
    os << "Time: " << state.time;
    return os;
}

bool operator==(const State& u, const State& v) {
    return u.minerals == v.minerals && u.robots == v.robots;
}

bool operator!=(const State& u, const State& v) {
    return !(u == v);
}

bool operator<(const State& u, const State& v) {
    return u.minerals < v.minerals || (u.minerals == v.minerals && u.robots < v.robots);
}

class BlueprintScorer {
public:
    BlueprintScorer(const Blueprint& blueprint, int time_limit)
        : m_blueprint(blueprint)
        , m_time_limit(time_limit)
        {}

    int score();

private:
    Blueprint m_blueprint;
    int m_time_limit;
    std::map<State, int> m_cache;

    int search(State& state);
};

int BlueprintScorer::score() {
    State state {Minerals{}, Robots{1, 0, 0, 0}, 0};
    return search(state);
}

int BlueprintScorer::search(State& state) {
    if (state.time >= m_time_limit) {
        m_cache[state] = state.minerals.geode;
        return state.minerals.geode;
    }
    auto it = m_cache.find(state);
    if (it != m_cache.end()) {
        if (it->first.time < state.time)
            return -1;
        else if (it->first.time == state.time)
            return it->second;
        else
            m_cache.erase(state);
    }
    int max_score = 0;

    if (state.minerals.ore >= m_blueprint.geode_ore && state.minerals.obsidian >= m_blueprint.geode_obsidian) {
        state.tick();
        state.minerals.ore      -= m_blueprint.geode_ore;
        state.minerals.obsidian -= m_blueprint.geode_obsidian;
        ++state.robots.geode;
        max_score = std::max(max_score, search(state));
        --state.robots.geode;
        state.minerals.ore      += m_blueprint.geode_ore;
        state.minerals.obsidian += m_blueprint.geode_obsidian;
        state.untick();
        m_cache[state] = max_score;
        return max_score;
    }
    if (state.minerals.ore >= m_blueprint.obsidian_ore &&
        state.minerals.clay >= m_blueprint.obsidian_clay &&
        state.robots.obsidian < m_blueprint.geode_obsidian)
    {
        state.tick();
        state.minerals.ore  -= m_blueprint.obsidian_ore;
        state.minerals.clay -= m_blueprint.obsidian_clay;
        ++state.robots.obsidian;
        max_score = std::max(max_score, search(state));
        --state.robots.obsidian;
        state.minerals.ore  += m_blueprint.obsidian_ore;
        state.minerals.clay += m_blueprint.obsidian_clay;
        state.untick();
        if (state.minerals.obsidian + state.robots.obsidian + 1 == m_blueprint.geode_obsidian &&
            state.minerals.ore + state.robots.ore - m_blueprint.obsidian_ore >= m_blueprint.geode_ore)
        {
            m_cache[state] = max_score;
            return max_score;
        }
    }
    if (state.minerals.ore >= m_blueprint.clay && state.robots.clay < m_blueprint.obsidian_clay) {
        state.tick();
        state.minerals.ore -= m_blueprint.clay;
        ++state.robots.clay;
        max_score = std::max(max_score, search(state));
        --state.robots.clay;
        state.minerals.ore += m_blueprint.clay;
        state.untick();
    }
    if (state.minerals.ore >= m_blueprint.ore &&
        state.robots.ore < std::max({m_blueprint.clay, m_blueprint.obsidian_ore, m_blueprint.geode_ore}))
    {
        state.tick();
        state.minerals.ore -= m_blueprint.ore;
        ++state.robots.ore;
        max_score = std::max(max_score, search(state));
        --state.robots.ore;
        state.minerals.ore += m_blueprint.ore;
        state.untick();
    }
    state.tick();
    max_score = std::max(max_score, search(state));
    state.untick();
    m_cache[state] = max_score;
    return max_score;
}

int part_one(const std::vector<Blueprint>& blueprints) {
    int sum = 0;
    for (const auto& blueprint : blueprints) {
        sum += BlueprintScorer{blueprint, 24}.score() * blueprint.id;
    }
    return sum;
}

int part_two(const std::vector<Blueprint>& blueprints) {
    int prod = 1;
    for (const auto& blueprint : blueprints) {
        prod *= BlueprintScorer{blueprint, 32}.score();
    }
    return prod;
}

int main() {
    for (auto file : {"sample.txt", "input.txt"}) {
        std::ifstream input {file};
        if (!input) {
            std::cerr << "Failed to open: " << file << '\n';
            return 1;
        }
        std::vector<Blueprint> blueprints;
        std::string line;
        while (std::getline(input, line)) {
            line.erase(std::remove_if(line.begin(), line.end(), [](char tok) -> bool {
                return !isspace(tok) && !std::isdigit(tok);
            }), line.end());
            std::istringstream iss {line};
            int id, cost_ore, cost_clay, cost_obsidian_ore, cost_obsidian_clay, cost_geode_ore, cost_geode_obsidian;
            iss >> id
                >> cost_ore
                >> cost_clay
                >> cost_obsidian_ore
                >> cost_obsidian_clay
                >> cost_geode_ore
                >> cost_geode_obsidian;
            Blueprint costs {id,
                             cost_ore,
                             cost_clay,
                             cost_obsidian_ore,
                             cost_obsidian_clay,
                             cost_geode_ore,
                             cost_geode_obsidian};
            blueprints.push_back(costs);
        }

        std::vector<Blueprint> first_blueprints;
        std::copy(blueprints.begin(),
                  blueprints.begin() + std::min(blueprints.size(), 3ULL),
                  std::back_inserter(first_blueprints));

        int ans_one = part_one(blueprints);
        int ans_two = part_two(first_blueprints);

        std::cout << file << ":\n";
        std::cout << "Answer part 1:  " << ans_one << '\n';
        std::cout << "Answer part 2:  " << ans_two << '\n';
    }
    return 0;
}
