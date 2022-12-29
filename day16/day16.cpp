#include <algorithm>
#include <fstream>
#include <iostream>
#include <iterator>
#include <map>
#include <numeric>
#include <queue>
#include <set>
#include <sstream>
#include <string>
#include <string_view>
#include <tuple>
#include <utility>
#include <vector>


struct Valve {
    int flow;
    std::string name;
    Valve() : Valve{"", 0} {}
    Valve(std::string_view name, int flow) : flow(flow), name(name) {}

    friend std::ostream& operator<<(std::ostream&, const Valve&);
};

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

struct TimeValve {
    Valve valve;
    int time;
    TimeValve(const Valve& valve, int time) : valve(valve), time(time) {}
    TimeValve(std::string_view name, int flow, int time) : TimeValve{Valve{name, flow}, time} {}
};

bool operator==(const TimeValve& a, const TimeValve& b) {
    return a.time == b.time && a.valve == b.valve;
}

bool operator!=(const TimeValve& a, const TimeValve& b) {
    return !(a == b);
}

bool operator<(const TimeValve& a, const TimeValve& b) {
    return b.time < a.time || (a.time == b.time && a.valve < b.valve);
}

template<std::size_t I>
std::tuple_element_t<I, TimeValve>& get(const TimeValve& time_valve) {
    if constexpr (I == 0) return time_valve.valve;
    if constexpr (I == 1) return time_valve.time;
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
    Vertex(std::string_view name, int flow, int time)
        : Vertex{Valve{name, flow}, time, {}}
        {}
    Vertex(std::string_view name, int flow, int time, const std::set<std::string>& activated)
        : Vertex{Valve{name, flow}, time, activated}
        {}

    friend std::ostream& operator<<(std::ostream&, const Vertex&);
};

std::ostream& operator<<(std::ostream& os, const Vertex& vertex) {
    os << vertex.valve << " at time " << vertex.time << " has weight " << vertex.weight;
    os << "\nhas activated: ";
    for (const auto& v : vertex.activated) {
        os << v << ' ';
    }
    return os;
}

bool operator==(const Vertex& a, const Vertex& b) {
    return a.weight == b.weight && a.time == b.time && a.valve == b.valve;
}

bool operator!=(const Vertex& a, const Vertex& b) {
    return !(a == b);
}

bool operator<(const Vertex& a, const Vertex& b) {
    if (a.weight < b.weight) return true;
    if (a.weight > b.weight) return false;
    return a.time < b.time || (a.time == b.time && a.valve < b.valve);
}

struct VertexPair {
    Vertex vertex_A;
    Vertex vertex_B;
    VertexPair(const Vertex& u, const Vertex& v) : vertex_A(u), vertex_B(v) {}

    int weight() const { return vertex_A.weight + vertex_B.weight; }
    bool is_activated(const std::string& name) const {
        return vertex_A.activated.count(name) || vertex_B.activated.count(name);
    }
    friend std::ostream& operator<<(std::ostream&, const VertexPair&);
};

std::ostream& operator<<(std::ostream& os, const VertexPair& dv) {
    os << "Human " << dv.vertex_A << '\n' << "Elephant " << dv.vertex_B;
    return os;
}

bool operator==(const VertexPair& a, const VertexPair& b) {
    return a.vertex_A == b.vertex_A && a.vertex_B == b.vertex_B;
}

bool operator!=(const VertexPair& a, const VertexPair& b) {
    return !(a == b);
}

bool operator<(const VertexPair& a, const VertexPair& b) {
    if (a.weight() < b.weight()) return true;
    if (a.weight() > b.weight()) return false;
    return a.vertex_A < b.vertex_A || (a.vertex_A == b.vertex_A && a.vertex_B < b.vertex_B);
}

using Graph         = std::map<Valve, std::vector<Valve>>;
using WeightedGraph = std::map<Valve, std::vector<TimeValve>>;

int single_dijkstra(const WeightedGraph& graph, const Valve& start, int time_limit) {
    std::map<TimeValve, int> weight;
    std::priority_queue<Vertex> pq;
    pq.emplace(start, time_limit);
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
            TimeValve node {adj.valve, next.time};
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

int double_dijkstra(const WeightedGraph& graph, const Valve& start_at, int time_limit) {
    std::map<std::pair<TimeValve, TimeValve>, int> weight;
    Vertex start {start_at, time_limit};
    std::priority_queue<VertexPair> pq;
    pq.emplace(start, start);

    const auto relevant_adjacent_valves = [&graph](const auto& vertex, const auto& vertex_pair)
    -> std::vector<TimeValve>
    {
        std::vector<TimeValve> ret;
        std::vector<TimeValve> adjs = graph.at(vertex.valve);
        std::copy_if(adjs.begin(),
                     adjs.end(),
                     std::back_inserter(ret),
                     [&vertex, &vertex_pair](const auto& adj) -> bool {
                         return vertex.time > adj.time && !vertex_pair.is_activated(adj.valve.name);
                     });
        return ret;
    };

    while (!pq.empty()) {
        VertexPair curr = pq.top();
        pq.pop();

        std::vector<TimeValve> filtered_adjs_A = relevant_adjacent_valves(curr.vertex_A, curr);
        std::vector<TimeValve> filtered_adjs_B = relevant_adjacent_valves(curr.vertex_B, curr);

        if (filtered_adjs_A.empty() && filtered_adjs_B.empty()) {
            continue;
        } else if (filtered_adjs_A.empty()) {
            for (const auto& [valve, time] : filtered_adjs_B) {
                Vertex next {valve, curr.vertex_B.time - time - 1};
                int w = next.weight + curr.weight();
                auto node = std::make_pair(TimeValve{curr.vertex_A.valve, curr.vertex_A.time},
                                           TimeValve{valve, next.time});
                if (w > weight[node]) {
                    next.activated = curr.vertex_B.activated;
                    next.activated.insert(valve.name);
                    next.weight  = curr.vertex_B.weight + next.weight;
                    weight[node] = w;
                    pq.emplace(curr.vertex_A, next);
                }
            }
        } else if (filtered_adjs_B.empty()) {
            for (const auto& [valve, time] : filtered_adjs_A) {
                Vertex next {valve, curr.vertex_A.time - time - 1};
                int w = next.weight + curr.weight();
                auto node = std::make_pair(TimeValve{valve, next.time},
                                           TimeValve{curr.vertex_B.valve, curr.vertex_B.time});
                if (w > weight[node]) {
                    next.activated = curr.vertex_A.activated;
                    next.activated.insert(valve.name);
                    next.weight  = curr.vertex_A.weight + next.weight;
                    weight[node] = w;
                    pq.emplace(next, curr.vertex_B);
                }
            }
        } else {
            for (const auto& [valve_A, time_A] : filtered_adjs_A) {
                for (const auto& [valve_B, time_B] : filtered_adjs_B) {
                    if (valve_A.name == valve_B.name) {
                        continue;
                    }
                    Vertex next_A {valve_A, curr.vertex_A.time - time_A - 1};
                    next_A.weight += curr.vertex_A.weight;

                    Vertex next_B {valve_B, curr.vertex_B.time - time_B - 1};
                    next_B.weight += curr.vertex_B.weight;

                    int w = next_A.weight + next_B.weight;
                    auto node = std::make_pair(TimeValve{valve_A, next_A.time},
                                               TimeValve{valve_B, next_B.time});
                    if (w > weight[node]) {
                        next_A.activated = curr.vertex_A.activated;
                        next_A.activated.insert(valve_A.name);
                        next_B.activated = curr.vertex_B.activated;
                        next_B.activated.insert(valve_B.name);
                        weight[node] = w;
                        pq.emplace(next_A, next_B);
                    }
                }
            }
        }
    }

    return std::accumulate(weight.cbegin(), weight.cend(), 0, [](int acc, const auto& w) -> int {
        return std::max(acc, w.second);
    });
}

/* Connect valves with flow > 0 by edges with weights corresponding to travel time. */
WeightedGraph compactify(const Graph& graph, const Valve& start) {
    WeightedGraph wgraph;

    const auto bfs = [&graph, &wgraph](const Valve& start) -> void {
        std::set<Valve> visited;
        std::queue<TimeValve> todo;
        todo.emplace(start, 0);
        while (!todo.empty()) {
            TimeValve u = todo.front();
            todo.pop();
            if (visited.count(u.valve)) {
                continue;
            }
            visited.insert(u.valve);
            if (u.valve != start && u.valve.flow > 0) {
                wgraph[start].push_back(u);
            }
            for (const auto& v : graph.at(u.valve)) {
                todo.emplace(v, u.time + 1);
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
            line.erase(line.begin());  // Remove V from Valve which was not erased
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
                graph[a].emplace_back(name_b, flow_map.at(name_b));
            }
        }

        WeightedGraph wgraph = compactify(graph, start);

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
