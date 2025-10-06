// ============================================================================
// Simulated Annealing (SA) para Maximum Independent Set (MIS)
// -----------------------------------------------------------------------------
// Descripción general:
//   Implementación de Recocido Simulado para el problema MIS en grafos
//   no dirigidos. Se parte desde una solución inicial maximal (Greedy por
//   grado creciente) y se aplican movimientos de intercambio controlados por
//   una temperatura T que decrece con factor α.
//
//   Comportamiento any-time: cada vez que se encuentra una mejor solución,
//   se imprime por salida estándar una línea con:
//       "<mejor_valor> <tiempo_desde_inicio_segundos>"
//   Al final de la ejecución (por tiempo), se repite esa mejor línea.
//
// Uso (CLI):
//   ./build/SA -i <graph_file> -t <seconds>
//              [--seed S] [--T0 1.0] [--alpha 0.999] [--iters_per_T N] [--check]
//
// Ejemplo:
//   ./build/SA -i data/.../erdos_n1000_p0c0.05_1.graph -t 5 --seed 1
//
// Entradas:
//   - Archivo de grafo (DIMACS-like: líneas "p edge n m" y "u v", o pares "u v")
//     soporta índices 0-based y 1-based (se detecta automáticamente).
//
// Salidas:
//   - En cada mejora:  "<best_value> <elapsed_seconds>"
//   - Al final:        misma línea repetida (mejor global).
//
// Compilación:
//   g++ -O3 -std=c++17 -DNDEBUG -Wall -o build/SA src/SA.cpp
//
// Notas de implementación:
//   • Estructura MIS_SA mantiene el estado de la solución, conflictos y mejor.
//   • Se completa a maximal después de aceptar un movimiento (fortalece any-time).
//   • --check valida independencia y maximalidad de la mejor solución final.
// ============================================================================

#include <bits/stdc++.h>
using namespace std;

// ----------------------------
// Estructura de grafo simple
// ----------------------------
struct Graph {
    int n = 0;
    vector<vector<int>> adj;
};

// -----------------------------------------------------------------------------
// read_graph(path)
// Lee un grafo no dirigido desde archivo. Soporta:
//   - Línea cabecera "p edge n m" (DIMACS-like)
//   - Pares "u v" uno por línea (0-based o 1-based)
// El detector one_based convierte a 0-based si corresponde. Elimina duplicados.
// Lanza std::runtime_error si no puede abrirse el archivo o si no hay datos.
// -----------------------------------------------------------------------------
Graph read_graph(const string& path) {
    ifstream fin(path);
    if (!fin) throw runtime_error("Cannot open file: " + path);

    string line;
    long long header_n = -1;
    vector<pair<int,int>> edges;
    long long max_node = -1;

    while (getline(fin, line)) {
        if (line.empty()) continue;
        // Trim leading spaces
        size_t i = 0; while (i < line.size() && isspace((unsigned char)line[i])) ++i;
        if (i >= line.size()) continue;

        char c = line[i];
        // Comentarios comunes en grafos
        if (c=='c' || c=='C' || c=='#' || c=='%' || c=='/') {
            continue;
        }
        if (c=='p' || c=='P') {
            // Espera: "p edge n m"
            string tag, kind;
            long long n=-1, m=-1;
            stringstream ss(line);
            ss >> tag >> kind >> n >> m;
            if (n>0) header_n = n;
            /*if (m>=0) header_m = m;*/
            continue;
        }
        // Intentar parsear dos enteros
        stringstream ss(line);
        long long a, b;
        if (ss >> a >> b) {
            edges.emplace_back((int)a, (int)b);
            max_node = max(max_node, max(a, b));
        } else {
            // Formatos no soportados (p.ej., listas de adyacencia), se ignoran
            continue;
        }
    }
    fin.close();

    int n = 0;
    if (header_n > 0) n = (int)header_n;
    else {
        // Inferir n desde el índice máximo observado
        if (max_node <= 0) throw runtime_error("Graph seems empty or unrecognized format.");
        n = (int)max_node;
    }

    // Detectar 1-based vs 0-based (heurística simple)
    bool has_zero = false, has_one = false;
    for (auto &e : edges) {
        if (e.first==0 || e.second==0) has_zero = true;
        if (e.first==1 || e.second==1) has_one = true;
    }
    bool one_based = (!has_zero && has_one);

    Graph G;
    G.n = n;
    G.adj.assign(n, {});

    auto add_edge_undirected = [&](int u, int v) {
        if (u==v) return;              // ignorar loops
        G.adj[u].push_back(v);
        G.adj[v].push_back(u);
    };

    for (auto &e : edges) {
        int u = e.first;
        int v = e.second;
        if (one_based) { u--; v--; }   // normalizar a 0-based si corresponde
        if (u<0 || v<0 || u>=n || v>=n) continue;
        add_edge_undirected(u,v);
    }

    // Deduplicar vecinos
    for (int u=0; u<n; ++u) {
        auto &nbr = G.adj[u];
        sort(nbr.begin(), nbr.end());
        nbr.erase(unique(nbr.begin(), nbr.end()), nbr.end());
    }
    return G;
}

// ----------------------------
// Temporizador simple (segundos)
// ----------------------------
struct Timer {
    chrono::steady_clock::time_point t0;
    Timer() { reset(); }
    void reset() { t0 = chrono::steady_clock::now(); }
    double elapsed() const {
        auto now = chrono::steady_clock::now();
        chrono::duration<double> diff = now - t0;
        return diff.count();
    }
};

// ============================================================================
// MIS_SA: estructura principal del Recocido Simulado para MIS
//   - inS[v]   : 1 si v ∈ S (solución actual), 0 en caso contrario
//   - conf[v]  : cantidad de vecinos de v que están en S (conflicto > 0)
//   - curr_size: |S| actual
//   - best_*   : mejor solución conocida y tiempo en que se halló
// ----------------------------------------------------------------------------
struct MIS_SA {
    const Graph& G;
    vector<char> inS;       // 1 si v está en S
    vector<int>  conf;      // conflictos: vecinos de v en S
    int curr_size = 0;

    int best_size = 0;
    vector<char> best_inS;
    double best_time = 0.0;

    mt19937_64 rng;
    uniform_real_distribution<double> U01;
    uniform_int_distribution<int> Uv;

    MIS_SA(const Graph& G_, uint64_t seed)
        : G(G_), inS(G_.n, 0), conf(G_.n, 0), best_inS(G_.n, 0),
          rng(seed), U01(0.0,1.0), Uv(0, max(0, G_.n-1)) {}

    // Agrega v a S y actualiza conflictos
    void add_vertex(int v) {
        inS[v] = 1;
        ++curr_size;
        for (int w : G.adj[v]) conf[w]++;
    }
    // Quita v de S y actualiza conflictos
    void remove_vertex(int v) {
        inS[v] = 0;
        --curr_size;
        for (int w : G.adj[v]) conf[w]--;
    }

    // Inicialización greedy maximal (orden por grado creciente)
    void init_greedy_maximal() {
        int n = G.n;
        vector<int> order(n);
        iota(order.begin(), order.end(), 0);
        sort(order.begin(), order.end(),
             [&](int a, int b){ return G.adj[a].size() < G.adj[b].size(); });

        for (int v : order) {
            if (!inS[v] && conf[v]==0) add_vertex(v);
        }
        best_size = curr_size;
        best_inS = inS;
        best_time = 0.0;
    }

    // Completar a maximal (añade cualquier v con conf[v]==0)
    void complete_to_maximal() {
        bool added = true;
        while (added) {
            added = false;
            for (int v=0; v<G.n; ++v) {
                if (!inS[v] && conf[v]==0) {
                    add_vertex(v);
                    added = true;
                }
            }
        }
    }

    // Registra mejor solución (any-time) e imprime línea “valor tiempo”
    void maybe_record_best(double elapsed_sec) {
        if (curr_size > best_size) {
            best_size = curr_size;
            best_inS = inS;
            best_time = elapsed_sec;
            cout.setf(std::ios::fixed);
            cout << setprecision(6) << best_size << " " << best_time << "\n";
            cout.flush();
        }
    }

    // Bucle principal de SA con detención por tiempo
    void run(double tmax, double T0, double alpha, int iters_per_T) {
        Timer tim;
        init_greedy_maximal();       // solución inicial factible y maximal
        maybe_record_best(0.0);

        double T = T0;
        if (G.n == 0) return;

        // Búsqueda con tiempo límite
        while (true) {
            double el = tim.elapsed();
            if (el >= tmax) break;

            // “Bloques” de iteraciones por temperatura
            for (int it=0; it<iters_per_T; ++it) {
                // Chequeo de tiempo cada ~1024 iteraciones
                if ((it & 1023) == 0) {
                    el = tim.elapsed();
                    if (el >= tmax) break;
                }

                // Proponemos agregar (con “swap” implícito): elegir v ∉ S
                int v = Uv(rng);
                if (inS[v]) continue;

                // k = cantidad de vecinos de v dentro de S
                int k = 0;
                for (int u : G.adj[v]) if (inS[u]) ++k;
                int delta = 1 - k;  // |S'| - |S| si saco N(v)∩S y meto v

                bool accept = (delta >= 0);
                if (!accept) {
                    double prob = exp((double)delta / max(1e-12, T));
                    if (U01(rng) < prob) accept = true;
                }

                if (accept) {
                    // Aplicar movimiento: quitar vecinos en S y agregar v
                    for (int u : G.adj[v]) if (inS[u]) remove_vertex(u);
                    add_vertex(v);

                    // Completar a maximal (mejora calidad any-time)
                    complete_to_maximal();
                    maybe_record_best(tim.elapsed());
                }
            }

            // Enfriamiento multiplicativo
            T *= alpha;
            if (T < 1e-12) T = 1e-12;
        }

        // Repetir mejor línea al final, como exige la pauta (any-time)
        cout.setf(std::ios::fixed);
        cout << setprecision(6) << best_size << " " << best_time << "\n";
        cout.flush();
    }
};

// -----------------------------------------------------------------------------
// Validadores de la mejor solución (opcionales, activados con --check)
//   - is_independent: no hay aristas internas en S
//   - is_maximal    : no se puede agregar ningún vértice sin romper independencia
// -----------------------------------------------------------------------------
static bool is_independent(const Graph& G, const vector<char>& inS) {
    for (int u = 0; u < G.n; ++u) if (inS[u]) {
        for (int v : G.adj[u]) if (inS[v]) return false;
    }
    return true;
}
static bool is_maximal(const Graph& G, const vector<char>& inS) {
    for (int u = 0; u < G.n; ++u) if (!inS[u]) {
        bool ok = true;
        for (int v : G.adj[u]) if (inS[v]) { ok = false; break; }
        if (ok) return false; // se podría agregar u ⇒ no es maximal
    }
    return true;
}

// ----------------------
// CLI y ejecución global
// ----------------------
struct Args {
    string   infile;
    double   tmax = 5.0;
    uint64_t seed = (uint64_t)chrono::high_resolution_clock::now().time_since_epoch().count();
    double   T0   = 1.0;
    double   alpha= 0.999;   // configuración por defecto (calibrada)
    int      iters_per_T = 1000;
    bool     do_check = false;
};

// Parseo simple de argumentos; aplica defaults seguros y valida rangos.
Args parse_args(int argc, char** argv) {
    Args a;
    for (int i=1; i<argc; ++i) {
        string s = argv[i];
        if      (s=="-i"            && i+1<argc) a.infile = argv[++i];
        else if (s=="-t"            && i+1<argc) a.tmax = stod(argv[++i]);
        else if (s=="--seed"        && i+1<argc) a.seed = stoull(argv[++i]);
        else if (s=="--T0"          && i+1<argc) a.T0 = stod(argv[++i]);
        else if (s=="--alpha"       && i+1<argc) a.alpha = stod(argv[++i]);
        else if (s=="--iters_per_T" && i+1<argc) a.iters_per_T = stoi(argv[++i]);
        else if (s=="--check") a.do_check = true;
        else {
            cerr << "Unknown or incomplete arg: " << s << "\n";
            exit(1);
        }
    }
    if (a.infile.empty()) {
        cerr << "Usage: SA -i <graph_file> -t <seconds> [--seed S] [--T0 1.0] [--alpha 0.999] [--iters_per_T N]\n";
        exit(1);
    }
    if (a.tmax <= 0) a.tmax = 1.0;
    if (a.T0   <= 0) a.T0   = 1.0;
    if (a.alpha <= 0 || a.alpha >= 1) a.alpha = 0.999;
    if (a.iters_per_T <= 0) a.iters_per_T = 1000;
    return a;
}

int main(int argc, char** argv) {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    try {
        Args args = parse_args(argc, argv);
        Graph G = read_graph(args.infile);

        // Escalado sensible de iters_per_T por tamaño del grafo (si quedó default)
        if (args.iters_per_T == 1000) {
            args.iters_per_T = max(1000, G.n);
        }

        MIS_SA solver(G, args.seed);
        solver.run(args.tmax, args.T0, args.alpha, args.iters_per_T);

        if (args.do_check) {
            bool indep = is_independent(G, solver.best_inS);
            bool maxm  = is_maximal(G, solver.best_inS);
            if (!indep || !maxm) {
                cerr << "[CHECK] ERROR: best_inS "
                     << (indep ? "" : "NO es independiente ")
                     << (maxm  ? "" : "NO es maximal ")
                     << "\n";
                return 2;
            }
        }
        return 0;
    } catch (const std::exception& e) {
        cerr << "ERROR: " << e.what() << "\n";
        return 1;
    }
}
