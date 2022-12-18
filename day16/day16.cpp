#include <algorithm>
#include <fstream>
#include <iostream>
#include <map>
#include <numeric>
#include <queue>
#include <set>
#include <sstream>
#include <string>
//#include <unordered_map>  // Maybe remove?
//#include <unordered_set>  // Maybe remove?
#include <vector>

struct Valve {
    int flow;
    std::string name;
    Valve() : Valve{"", 0} {}
    Valve(const std::string& name, int flow) : flow(flow), name(name) {}

    friend std::ostream& operator<<(std::ostream& os, const Valve& valve);
};

using Graph = std::map<Valve, std::vector<Valve>>;

std::ostream& operator<<(std::ostream& os, const Valve& valve) {
    os << '(' << valve.name << ',' << valve.flow << ')';
    return os;
}

bool operator==(const Valve& a, const Valve& b) {
    return a.name == b.name;
}

bool operator!=(const Valve& a, const Valve& b) {
    return !(a == b);
}

bool operator<(const Valve& a, const Valve& b) {
    return a.flow < b.flow || (a.flow == b.flow && a.name < b.name);
}

template<>
struct std::hash<Valve> {
    std::size_t operator()(Valve const& valve) const noexcept {
        std::size_t h1 = std::hash<int>{}(valve.flow);
        std::size_t h2 = std::hash<std::string>{}(valve.name);
        return h1 ^ (h2 << 1);
    }
};

struct TValve {
    Valve valve;
    int time;
    TValve(const Valve& valve, int time) : valve(valve), time(time) {}
    TValve(const std::string& name, int flow, int time) : TValve{Valve{name, flow}, time} {}
};

using WGraph = std::map<Valve, std::vector<TValve>>;

bool operator==(const TValve& a, const TValve& b) {
    return a.time == b.time && a.valve == b.valve;
}

bool operator!=(const TValve& a, const TValve& b) {
    return !(a == b);
}

bool operator<(const TValve& a, const TValve& b) {
    return b.time < a.time || (a.time == b.time && a.valve < b.valve);
}

struct Vertex {
    Valve valve;
    int time;
    int weight;
    std::set<std::string> activated;
    Vertex(const Valve& valve, int time, const std::set<std::string>& activated)
        : valve(valve)
        , time(time)
        , weight(time * valve.flow)
        , activated(activated)
        {}
    Vertex(const Valve& valve, int time)
        : Vertex{valve, time, {}}
        {}
    Vertex(const std::string& name, int flow, int time)
        : Vertex{Valve{name, flow}, time, {}}
        {}
    Vertex(const std::string& name, int flow, int time, const std::set<std::string>& activated)
        : Vertex{Valve{name, flow}, time, activated}
        {}

    friend std::ostream& operator<<(std::ostream& os, const Vertex& vertex);
};

std::ostream& operator<<(std::ostream& os, const Vertex& vertex) {
    os << vertex.valve << " at time " << vertex.time << " has weight " << vertex.weight;
    os << "\nhas used up: ";
    for (const auto& v : vertex.activated) {
        os << v << ' ';
    }
    return os;
}

bool operator<(const Vertex& a, const Vertex& b) {
    return a.weight < b.weight;  // If don't work, compare time and valves as well.
}

struct DTValve {
    TValve tv1;
    TValve tv2;
    DTValve(const TValve& a, const TValve& b) : tv1(a), tv2(b) {}
};

bool operator<(const DTValve& a, const DTValve& b) {
    return a.tv1 < b.tv1 || (a.tv1 == b.tv1 && a.tv2 < b.tv2);
}

struct DVertex {
    Vertex v1;
    Vertex v2;
    DVertex(const Vertex& u, const Vertex& v) : v1(u), v2(v) {}

    int weight() const { return v1.weight + v2.weight; }
    bool isActivated(const std::string& name) const { return v1.activated.count(name) || v2.activated.count(name); }

    friend std::ostream& operator<<(std::ostream& os, const DVertex& dv);
};

std::ostream& operator<<(std::ostream& os, const DVertex& dv) {
    os << "Human " << dv.v1 << '\n' << "Elephant " << dv.v2;
    return os;
}

bool operator<(const DVertex& a, const DVertex& b) {
    return a.weight() < b.weight();
}


int single_dijkstra(const WGraph& graph, const Valve& start_at, int time_limit) {
    std::map<TValve, int> weight;
    Vertex start {start_at, time_limit};
    std::priority_queue<Vertex> pq;
    pq.push(start);
    while (!pq.empty()) {
        Vertex curr = pq.top();
        pq.pop();
        if (curr.time <= 0) {
            continue;
        }
        for (const auto& adj : graph.at(curr.valve)) {
            if (curr.time <= adj.time || curr.activated.count(adj.valve.name)) {
                continue;
            }
            Vertex next {adj.valve, curr.time - adj.time - 1};
            int w = next.weight + curr.weight;
            TValve node {adj.valve, next.time};
            if (w > weight[node]) {
                next.activated = curr.activated;
                next.activated.insert(adj.valve.name);
                next.weight  = w;
                weight[node] = w;
                pq.push(next);
            }
        }
    }

    return std::accumulate(weight.cbegin(), weight.cend(), 0, [](int acc, const auto& w) -> int {
        return std::max(acc, w.second);
    });
}

int double_dijkstra(const WGraph& graph, const Valve& start_at, int time_limit) {
    std::map<DTValve, int> weight;
    Vertex start {start_at, time_limit};
    std::priority_queue<DVertex> pq;
    pq.emplace(DVertex{start, start});

    // const auto double_activation = [](const DVertex& dv) -> bool {
    //     for (const auto& n : dv.v1.activated) {
    //         if (dv.v2.activated.count(n)) {
    //             return true;
    //         }
    //     }
    //     return false;
    // };

    while (!pq.empty()) {
        DVertex curr = pq.top();
        pq.pop();

        std::vector<TValve> filtered_adjs_1;
        std::vector<TValve> adjs_1 = graph.at(curr.v1.valve);
        std::copy_if(adjs_1.begin(), adjs_1.end(), std::back_inserter(filtered_adjs_1), [&curr](const auto& adj) -> bool {
            return curr.v1.time > adj.time && !curr.isActivated(adj.valve.name);
        });
        std::vector<TValve> filtered_adjs_2;
        std::vector<TValve> adjs_2 = graph.at(curr.v2.valve);
        std::copy_if(adjs_2.begin(), adjs_2.end(), std::back_inserter(filtered_adjs_2), [&curr](const auto& adj) -> bool {
            return curr.v2.time > adj.time && !curr.isActivated(adj.valve.name);
        });
        bool can_move_1 = !filtered_adjs_1.empty();
        bool can_move_2 = !filtered_adjs_2.empty();

        if (!can_move_1 && !can_move_2) {
            continue;
        } else if (!can_move_1) {
            for (const auto& adj : filtered_adjs_2) {
                Vertex next {adj.valve, curr.v2.time - adj.time - 1};
                int w = next.weight + curr.weight();
                DTValve node {TValve{curr.v1.valve, curr.v1.time}, TValve{adj.valve, next.time}};
                if (w > weight[node]) {
                    next.activated = curr.v2.activated;
                    next.activated.insert(adj.valve.name);
                    next.weight  = curr.v2.weight + next.weight;
                    weight[node] = w;
                    pq.push(DVertex{curr.v1, next});
                }
            }
        } else if (!can_move_2) {
            for (const auto& adj : filtered_adjs_1) {
                Vertex next {adj.valve, curr.v1.time - adj.time - 1};
                int w = next.weight + curr.weight();
                DTValve node {TValve{adj.valve, next.time}, TValve{curr.v2.valve, curr.v2.time}};
                if (w > weight[node]) {
                    next.activated = curr.v1.activated;
                    next.activated.insert(adj.valve.name);
                    next.weight  = curr.v1.weight + next.weight;
                    weight[node] = w;
                    pq.push(DVertex{next, curr.v2});
                }
            }
        } else {
            for (const auto& adj_1 : filtered_adjs_1) {
                for (const auto& adj_2 : filtered_adjs_2) {
                    if (adj_1.valve.name == adj_2.valve.name) {
                        continue;
                    }
                    Vertex next_1 {adj_1.valve, curr.v1.time - adj_1.time - 1};
                    next_1.weight += curr.v1.weight;

                    Vertex next_2 {adj_2.valve, curr.v2.time - adj_2.time - 1};
                    next_2.weight += curr.v2.weight;

                    int w = next_1.weight + next_2.weight;
                    DTValve node {TValve{adj_1.valve, next_1.time}, TValve{adj_2.valve, next_2.time}};
                    if (w > weight[node]) {
                        next_1.activated = curr.v1.activated;
                        next_1.activated.insert(adj_1.valve.name);
                        next_2.activated = curr.v2.activated;
                        next_2.activated.insert(adj_2.valve.name);
                        DVertex next {next_1, next_2};
                        weight[node] = w;
                        pq.push(next);
                    }
                }
            }
        }
    }

    return std::accumulate(weight.cbegin(), weight.cend(), 0, [](int acc, const auto& w) -> int {
        return std::max(acc, w.second);
    });
}

WGraph compactify(const Graph& graph, const Valve& start) {
    WGraph wgraph;

    const auto bfs = [&graph, &wgraph](const Valve& start) -> void {
        std::set<Valve> visited;
        std::queue<TValve> todo;
        todo.push(TValve{start, 0});
        while (!todo.empty()) {
            TValve u = todo.front();
            todo.pop();
            if (visited.count(u.valve)) {
                continue;
            }
            visited.insert(u.valve);
            if (u.valve != start && u.valve.flow > 0) {
                wgraph[start].push_back(u);
            }
            for (const auto& v : graph.at(u.valve)) {
                todo.emplace(TValve{v, u.time + 1});
            }
        }
    };

    bfs(start);
    for (const auto& [valve, connections] : graph) {
        if (valve.flow > 0) {
            bfs(valve);
        }
    }
    return wgraph;
}

int main() {
    for (auto file : {"sample.txt", "input.txt"}) {
        std::ifstream input {file};
        if (!input) {
            std::cerr << "Failed to open: " << file << '\n';
            return 1;
        }
        std::map<std::string, std::vector<std::string>> adj;
        std::map<std::string, int> flow_map;
        std::string line;
        while (std::getline(input, line)) {
            line.erase(std::remove_if(line.begin(), line.end(), [](char tok) -> bool {
                return !std::isspace(tok) && !std::isupper(tok) && !std::isdigit(tok);
            }), line.end());
            line.erase(line.begin());  // Remove V from Valve
            std::istringstream iss {line};
            std::string name, connected;
            int flow;
            iss >> name >> flow;
            flow_map[name] = flow;
            while (iss >> connected) {
                adj[name].push_back(connected);
            }
        }

        Valve start {"AA", flow_map.at("AA")};
        Graph graph;
        for (const auto& [name_a, connected] : adj) {
            Valve a {name_a, flow_map.at(name_a)};
            for (const auto& name_b : connected) {
                Valve b {name_b, flow_map.at(name_b)};
                graph[a].emplace_back(b);
            }
        }

        WGraph wgraph = compactify(graph, start);

        int ans_one = single_dijkstra(wgraph, start, 30);
        int ans_two = double_dijkstra(wgraph, start, 26);

        std::cout << file << ":\n";
        std::cout << "Answer part 1:  " << ans_one << '\n';
        std::cout << "Answer part 2:  " << ans_two << '\n';
    }
    return 0;
}

/*
 * sample.txt:
 * Answer part 1:  1651
 * Answer part 2:  1707
 * input.txt:
 * Answer part 1:  1850
 * Answer part 2:  2306
 */
