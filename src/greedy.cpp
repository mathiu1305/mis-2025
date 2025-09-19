// ~/mis-2025/src/greedy.cpp
#include <iostream>
#include <iomanip>
#include <vector>
#include <climits>
#include "graph_io.hpp"
#include "utils.hpp"

int main(int argc, char** argv) {
    std::string in_path;
    for (int i = 1; i < argc; ++i) {
        std::string a = argv[i];
        if ((a == "-i" || a == "--input") && i + 1 < argc) in_path = argv[++i];
    }
    if (in_path.empty()) return 1;

    Graph G = load_graph(in_path);
    const int n = G.n;

    std::vector<char> alive(n, 1);
    std::vector<int> deg(n);
    for (int u = 0; u < n; ++u) deg[u] = (int)G.adj[u].size();

    int solution_size = 0;
    int alive_count = n;

    double t0 = now_seconds();
    while (alive_count > 0) {
        int best = -1, best_deg = INT_MAX;
        for (int u = 0; u < n; ++u) if (alive[u]) {
            if (deg[u] < best_deg) { best_deg = deg[u]; best = u; }
        }
        if (best == -1) break;

        ++solution_size;

        std::vector<int> to_remove;
        to_remove.reserve(1 + G.adj[best].size());
        to_remove.push_back(best);
        for (int v : G.adj[best]) if (alive[v]) to_remove.push_back(v);

        for (int r : to_remove) if (alive[r]) {
            alive[r] = 0; --alive_count;
            for (int w : G.adj[r]) if (alive[w]) --deg[w];
        }
    }
    double elapsed = now_seconds() - t0;

    std::cout << solution_size << " " << std::fixed << std::setprecision(6) << elapsed << "\n";
    return 0;
}
