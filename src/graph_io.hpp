// ~/mis-2025/src/graph_io.hpp
#pragma once
#include <vector>
#include <string>
#include <fstream>
#include <stdexcept>

struct Graph {
    int n = 0;
    long long m = 0;
    std::vector<std::vector<int>> adj; // 0-based
};

inline Graph load_graph(const std::string& path) {
    std::ifstream in(path);
    if (!in) throw std::runtime_error("No se pudo abrir: " + path);

    Graph G;
    if (!(in >> G.n) || G.n <= 0)
        throw std::runtime_error("Encabezado inválido en: " + path);

    G.adj.assign(G.n, {});
    long long u, v;
    while (in >> u >> v) {
        if (u == v) continue; // ignorar lazos
        if (u < 0 || v < 0 || u >= G.n || v >= G.n)
            throw std::runtime_error("Índice fuera de rango en: " + path);
        G.adj[(int)u].push_back((int)v);
        G.adj[(int)v].push_back((int)u);
        ++G.m;
    }
    return G;
}
