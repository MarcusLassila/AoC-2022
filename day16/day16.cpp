#include <bits/stdc++.h>

static constexpr int time_limit = 30U;

struct Int {
    int value;
    Int() : value(-1) {}
    Int(int value) : value(value) {}

    operator int() const { return value; }
};

struct Valve {
    int flow;
    std::string name;
    Valve() : flow(0) {}
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

struct TValve {
    Valve valve;
    int time;
    TValve(const Valve& valve, int time) : valve(valve), time(time) {}
    TValve(const std::string& name, int flow, int time) : valve(Valve{name, flow}), time(time) {}
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
    Vertex(const Valve& valve, int time)
        : valve(valve)
        , time(time)
        , weight(time * valve.flow)
        {}
    Vertex(const std::string& name, int flow, int time)
        : valve(Valve{name, flow})
        , time(time)
        , weight(time * flow)
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

int dijkstra(const WGraph& graph) {
    std::map<TValve, int> weight;
    Vertex start {"AA", 0, 30};
    std::priority_queue<Vertex> pq;
    pq.push(start);
    while (!pq.empty()) {
        Vertex curr = pq.top();
        //std::cout << "Pop vertex: " << curr << '\n';
        pq.pop();
        if (curr.time <= 0) {
            continue;
        }
        for (const auto& adj : graph.at(curr.valve)) {
            if (curr.time > adj.time && adj.valve.flow > 0 && !curr.activated.count(adj.valve.name)) {
                Vertex next {adj.valve, curr.time - adj.time - 1};
                int w = next.weight + curr.weight;
                TValve node {adj.valve, next.time};
                if (w > weight[node]) {
                    next.activated = curr.activated;
                    next.activated.insert(adj.valve.name);
                    next.weight = w;
                    weight[node] = w;
                    pq.push(next);
                }
            }
        }
    }
    int max_weight = 0;
    for (const auto& [n, w] : weight) {
        max_weight = std::max(max_weight, w);
    }
    return max_weight;
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

int double_dijkstra(const WGraph& graph) {
    std::map<DTValve, int> weight;
    Vertex start {"AA", 0, 26};
    std::priority_queue<DVertex> pq;
    pq.emplace(DVertex{start, start});

    const auto can_move = [&](const DVertex& dv) -> std::pair<bool, bool> {
        std::vector<TValve> adjs = graph.at(dv.v1.valve);
        bool a = std::any_of(adjs.begin(), adjs.end(), [&](const TValve& tv) -> bool {
            return dv.v1.time > tv.time && !dv.isActivated(tv.valve.name);
        });
        adjs = graph.at(dv.v2.valve);
        bool b = std::any_of(adjs.begin(), adjs.end(), [&](const TValve& tv) -> bool {
            return dv.v2.time > tv.time && !dv.isActivated(tv.valve.name);
        });
        return {a, b};
    };

    const auto double_activation = [](const DVertex& dv) -> bool {
        for (const auto& n : dv.v1.activated) {
            if (dv.v2.activated.count(n)) {
                return true;
            }
        }
        return false;
    };

    while (!pq.empty()) {
        DVertex curr = pq.top();
        
        pq.pop();
        const auto& [can_move_1, can_move_2] = can_move(curr);
        if (!can_move_1 && !can_move_2) {
            continue;
        } else if (!can_move_1) {
            for (const auto& adj : graph.at(curr.v2.valve)) {
                if (curr.v2.time > adj.time && adj.valve.flow > 0 && !curr.isActivated(adj.valve.name)) {
                    Vertex next {adj.valve, curr.v2.time - adj.time - 1};
                    int w = next.weight + curr.weight();
                    DTValve node {TValve{curr.v1.valve, curr.v1.time}, TValve{adj.valve, next.time}};
                    if (w > weight[node]) {
                        next.activated = curr.v2.activated;
                        next.activated.insert(adj.valve.name);
                        next.weight = curr.v2.weight + next.weight;
                        weight[node] = w;
                        pq.push(DVertex{curr.v1, next});
                    }
                }
            }
        } else if (!can_move_2) {
            for (const auto& adj : graph.at(curr.v1.valve)) {
                if (curr.v1.time > adj.time && adj.valve.flow > 0 && !curr.isActivated(adj.valve.name)) {
                    Vertex next {adj.valve, curr.v1.time - adj.time - 1};
                    int w = next.weight + curr.weight();
                    DTValve node {TValve{adj.valve, next.time}, TValve{curr.v2.valve, curr.v2.time}};
                    if (w > weight[node]) {
                        next.activated = curr.v1.activated;
                        next.activated.insert(adj.valve.name);
                        next.weight = curr.v1.weight + next.weight;
                        weight[node] = w;
                        pq.push(DVertex{next, curr.v2});
                    }
                }
            }
        } else {
            for (const auto& adj_1 : graph.at(curr.v1.valve)) {
                if (curr.v1.time <= adj_1.time || curr.isActivated(adj_1.valve.name)) {
                    continue;
                }
                Vertex next_1 {adj_1.valve, curr.v1.time - adj_1.time - 1};
                next_1.activated = curr.v1.activated;
                next_1.activated.insert(adj_1.valve.name);
                next_1.weight += curr.v1.weight;
                for (const auto& adj_2 : graph.at(curr.v2.valve)) {
                    if (adj_1.valve.name == adj_2.valve.name || curr.v2.time <= adj_2.time || curr.isActivated(adj_2.valve.name)) {
                        continue;
                    }
                    Vertex next_2 {adj_2.valve, curr.v2.time - adj_2.time - 1};
                    next_2.activated = curr.v2.activated;
                    next_2.activated.insert(adj_2.valve.name);
                    next_2.weight += curr.v2.weight;
                    DVertex next {next_1, next_2};
                    int w = next.weight();
                    DTValve node {TValve{adj_1.valve, next_1.time}, TValve{adj_2.valve, next_2.time}};
                    if (w > weight[node]) {
                        weight[node] = w;
                        pq.push(next);
                    }
                }
            }
        }
    }

    int max_weight = 0;
    for (const auto& [n, w] : weight) {
        max_weight = std::max(max_weight, w);
    }
    return max_weight;
}

void bfs(const Graph& graph, WGraph& wgraph, const Valve& start) {
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
        if (!(u.valve == start) && u.valve.flow > 0) {
            wgraph[start].push_back(u);
        }
        for (const auto& v : graph.at(u.valve)) {
            todo.emplace(TValve{v, u.time + 1});
        }
    }
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
            std::istringstream iss {line};
            std::string name, connected;
            int flow;
            iss >> name >> name >> flow;
            flow_map[name] = flow;
            while (iss >> connected) {
                adj[name].push_back(connected);
            }
        }

        Graph graph;
        for (const auto& [name_a, connected] : adj) {
            Valve a {name_a, flow_map.at(name_a)};
            for (const auto& name_b : connected) {
                Valve b {name_b, flow_map.at(name_b)};
                graph[a].emplace_back(b);
            }
        }

        WGraph wgraph;
        bfs(graph, wgraph, Valve{"AA", 0});
        for (const auto& [u, vs] : graph) {
            if (u.flow > 0) {
                bfs(graph, wgraph, u);
            }
        }
        // for (const auto& [u, vs] : wgraph) {
        //     std::cout << u << " has adjacent nodes:\n";
        //     for (const auto& v : vs) {
        //         std::cout << v.valve << " at time " << v.time << ' ';
        //     }
        //     std::cout << '\n';
        // }

        int ans_one = dijkstra(wgraph);
        int ans_two = double_dijkstra(wgraph);

        std::cout << file << ":\n";
        std::cout << "Answer part 1:  " << ans_one << '\n';
        std::cout << "Answer part 2:  " << ans_two << '\n';
    }
    return 0;
}




// int dijkstra2(const Graph& graph) {

//     std::map<TValve, int> weight;
//     // for (const auto& pair : graph) {
//     //     weight[pair.first.name] = 0;
//     // }
//     std::string start = "AA";
//     std::priority_queue<Vertex> pq;
//     pq.push(Vertex(start, 0, 30));
//     //std::set<Vertex> processed;
//     //weight[Node{start, 30}].value = 0;
//     while (!pq.empty()) {
//         Vertex curr = pq.top();
//         std::cout << "Pop vertex: " << curr << '\n';
//         pq.pop();
//         if (curr.time <= 2) {
//             continue;
//         }
//         // if (processed.count(curr)) {
//         //     continue;
//         // }
//         // processed.insert(curr);
//         for (const auto& adj : graph.at(curr.valve)) {
//             Vertex next {adj, curr.time - 1};
//             int w = curr.weight;
//             TValve node {adj, next.time};
//             if (w > weight[node]) {
//                 next.activated = curr.activated;
//                 next.weight = w;
//                 weight[node] = w;
//                 pq.push(next);
//             }
//             if (adj.flow > 0 && std::find(curr.activated.begin(), curr.activated.end(), adj.name) == curr.activated.end()) { //!curr.activated.count(adj.name)) {
//                 Vertex next {adj, curr.time - 2};
//                 int w = next.weight + curr.weight;
//                 TValve node {adj, next.time};
//                 if (w > weight[node]) {
//                     next.activated = curr.activated;
//                     next.activated.push_back(adj.name);
//                     next.weight = w;
//                     weight[node] = w;
//                     pq.push(next);
//                 }
//             }
//         }
//     }
//     int max_weight = 0;
//     for (const auto& [n, w] : weight) {
//         //std::cout << n.name << " weight: " << w.value << '\n'; 
//         max_weight = std::max(max_weight, w);
//     }
//     return max_weight;
// }