#include <iostream>
#include <iomanip>
#include <vector>
#include <random>
#include <climits>
#include "graph_io.hpp"
#include "utils.hpp"

/**
 * Heurística greedy aleatorizada para MIS (RCL-α):
 *  - Calcula umbral = d_min + α (d_max - d_min).
 *  - RCL = {u vivos con grado[u] <= umbral}.
 *  - Elige u aleatorio en RCL, lo agrega a la solución y elimina u y vecinos.
 * Parámetros:
 *  -i / --input <instancia.graph>
 *  --alpha <0..1>     (default 0.3)
 *  --seed  <entero>   (default 12345)
 * Salida (stdout): "<valor> <tiempo>"
 */
int main(int argc, char** argv) {
    std::string in_path; double alpha = 0.3; unsigned seed = 12345;
    for (int i = 1; i < argc; ++i) {
        std::string a = argv[i];
        if ((a == "-i" || a == "--input") && i + 1 < argc) in_path = argv[++i];
        else if (a == "--alpha" && i + 1 < argc) alpha = std::stod(argv[++i]);
        else if (a == "--seed"  && i + 1 < argc) seed  = (unsigned)std::stoul(argv[++i]);
    }
    if (in_path.empty()) return 1;
    if (alpha < 0.0) alpha = 0.0;
    if (alpha > 1.0) alpha = 1.0;

    Graph G = load_graph(in_path);
    const int n = G.n;

    std::vector<char> alive(n, 1);
    std::vector<int> deg(n);
    for (int u = 0; u < n; ++u) deg[u] = (int)G.adj[u].size();

    std::mt19937 rng(seed);
    int solution_size = 0;
    int alive_count = n;

    double t0 = now_seconds();
    while (alive_count > 0) {
        // Rango de grados en los nodos vivos
        int dmin = INT_MAX, dmax = -1;
        for (int u = 0; u < n; ++u) if (alive[u]) {
            if (deg[u] < dmin) dmin = deg[u];
            if (deg[u] > dmax) dmax = deg[u];
        }
        if (dmin == INT_MAX) break;

        // Umbral para construir RCL
        const double thr = dmin + alpha * (double)(dmax - dmin);

        std::vector<int> RCL;
        RCL.reserve(64);
        for (int u = 0; u < n; ++u) if (alive[u]) {
            if ((double)deg[u] <= thr) RCL.push_back(u);
        }
        // Salvaguarda: si RCL queda vacía, agregamos un vivo cualquiera
        if (RCL.empty()) {
            for (int u = 0; u < n; ++u) if (alive[u]) { RCL.push_back(u); break; }
        }

        // Elección aleatoria dentro de RCL
        std::uniform_int_distribution<int> dist(0, (int)RCL.size() - 1);
        int chosen = RCL[dist(rng)];

        ++solution_size;

        // Eliminación del elegido y de sus vecinos; actualización de grados
        std::vector<int> to_remove;
        to_remove.reserve(1 + G.adj[chosen].size());
        to_remove.push_back(chosen);
        for (int v : G.adj[chosen]) if (alive[v]) to_remove.push_back(v);

        for (int r : to_remove) if (alive[r]) {
            alive[r] = 0; --alive_count;
            for (int w : G.adj[r]) if (alive[w]) --deg[w];
        }
    }
    double elapsed = now_seconds() - t0;

    std::cout << solution_size << " "
              << std::fixed << std::setprecision(6) << elapsed << "\n";
    return 0;
}
