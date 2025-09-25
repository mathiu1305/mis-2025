#pragma once
#include <vector>
#include <string>
#include <fstream>
#include <stdexcept>

/**
 * Representación básica de grafo no dirigido con listas de adyacencia.
 * - n: número de nodos (0..n-1)
 * - m: número de aristas (se incrementa por cada par u v leído)
 * - adj[u]: vecinos de u
 */
struct Graph {
    int n = 0;
    long long m = 0;
    std::vector<std::vector<int>> adj; // 0-based
};

/**
 * Carga un grafo desde archivo .graph con el formato:
 *  1) Primera línea: n
 *  2) Resto: pares "u v" (0-based), una arista por línea
 * Ignora lazos (u==v) y valida rangos de índices.
 * Lanza std::runtime_error si hay problemas de lectura.
 */
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
