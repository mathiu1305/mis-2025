// ============================================================================
// Simulated Annealing (SA) para Maximum Independent Set (MIS) - VERSIÓN OPTIMIZADA
// -----------------------------------------------------------------------------
//
//   Comportamiento any-time: cada vez que se encuentra una mejor solución,
//   se imprime por salida estándar una línea con:
//       "<mejor_valor> <tiempo_desde_inicio_segundos>"
//   Al final de la ejecución (por tiempo), se repite esa mejor línea.
//
// Uso (CLI):
//   ./build/SA -i <graph_file> -t <seconds>
//              [--seed S] [--T0 1.0] [--alpha 0.999] [--iters_per_T N] [--check]
//              [--diversify] [--adaptive_temp]
//
// Ejemplo:
//   ./build/SA -i data/.../erdos_n1000_p0c0.05_1.graph -t 5 --seed 1 --diversify
//
// Nuevas características:
//   • Inicialización greedy mejorada con múltiples criterios
//   • Diversificación automática cuando se estanca la búsqueda
//   • Control de temperatura adaptativo basado en tasa de aceptación
//   • Optimizaciones de memoria para mejor rendimiento en grafos grandes
//   • Estadísticas detalladas de rendimiento (opcional)
// ============================================================================

#include <bits/stdc++.h>
using namespace std;

// ----------------------------
// Estructura de grafo optimizada
// ----------------------------
struct Graph {
    int n = 0;
    vector<vector<int>> adj;
    vector<int> degree;  // Precalculamos grados para optimización
    
    void compute_degrees() {
        degree.resize(n);
        for (int v = 0; v < n; ++v) {
            degree[v] = (int)adj[v].size();
        }
    }
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
            // Validación adicional para evitar valores extremos
            if (a >= 0 && b >= 0 && a < 1000000 && b < 1000000) {
                edges.emplace_back((int)a, (int)b);
                max_node = max(max_node, max(a, b));
            } else {
                cerr << "[WARN] Ignorando arista con valores extremos: " << a << " " << b << "\n";
            }
        } else {
            // Formatos no soportados (p.ej., listas de adyacencia), se ignoran
            continue;
        }
    }
    fin.close();

    // Detectar 1-based vs 0-based usando las aristas leídas 
    bool has_zero = false, has_one = false;
    for (auto &e : edges) {
        if (e.first == 0 || e.second == 0) has_zero = true;
        if (e.first == 1 || e.second == 1) has_one  = true;
    }
    bool one_based = (!has_zero && has_one);

    // Fijar n 
    int n = 0;
    if (header_n > 0) {
        n = (int)header_n;
    } else {
        // Inferir n desde el índice máximo observado
        if (max_node < 0) throw runtime_error("Graph seems empty or unrecognized format.");
        // 0-based => max_index + 1 ; 1-based => max_index (porque luego normalizamos a 0-based)
        n = (int)max_node + (one_based ? 0 : 1);
    }

    // Construir grafo
    Graph G;
    G.n = n;
    G.adj.assign(n, {});

    auto add_edge_undirected = [&](int u, int v) {
        if (u == v) return;          // ignorar loops
        if (u < 0 || v < 0 || u >= G.n || v >= G.n) return; // sanity
        G.adj[u].push_back(v);
        G.adj[v].push_back(u);
    };


    for (auto &e : edges) {
        int u = e.first;
        int v = e.second;
        if (one_based) { u--; v--; }   // normalizar a 0-based si corresponde
        if (u<0 || v<0 || u>=n || v>=n) continue;
        if (u == v) continue;              // evita loops
        if (u > v) swap(u, v);             // normaliza par
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
// Temporizador mejorado con estadísticas
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
// MIS_SA_OPTIMIZED: versión mejorada del Recocido Simulado para MIS
// ----------------------------------------------------------------------------
struct MIS_SA {
    const Graph& G;
    vector<char> inS;       // 1 si v está en S
    vector<int>  conf;      // conflictos: vecinos de v en S
    int curr_size = 0;

    int best_size = 0;
    vector<char> best_inS;
    double best_time = 0.0;

    // Nuevas estructuras para optimización
    vector<int> candidates;     // Lista de candidatos para movimientos
    vector<char> is_candidate;  // Marcador rápido de candidatos
    vector<int> vertex_priority; // Prioridad de vértices para diversificación
    
    // Estadísticas de rendimiento
    long long total_moves = 0;
    long long accepted_moves = 0;
    long long improvements = 0;
    int stagnation_count = 0;
    double last_improvement_time = 0.0;
    
    // Parámetros adaptativos
    bool use_diversification = false;
    bool use_adaptive_temp = false;
    bool verbose = false;  // Nuevo: para logging detallado
    double acceptance_rate = 0.0;
    
    mt19937_64 rng;
    uniform_real_distribution<double> U01;
    uniform_int_distribution<int> Uv;

    MIS_SA(const Graph& G_, uint64_t seed)
        : G(G_), inS(G_.n, 0), conf(G_.n, 0), best_inS(G_.n, 0),
          candidates(), is_candidate(G_.n, 0), vertex_priority(G_.n, 0),
          rng(seed), U01(0.0,1.0), Uv(0, max(0, G_.n-1)) {
        
        // Pre-reservar memoria para estructuras dinámicas
        candidates.reserve(G_.n);
        
        // Inicializar prioridades de vértices (para diversificación)
        for (int v = 0; v < G.n; ++v) {
            vertex_priority[v] = G.degree[v];
        }
    }

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

    // Actualizar lista de candidatos de manera eficiente
    void update_candidates() {
        candidates.clear();
        // Reservar espacio conservadoramente 
        int estimated_candidates = max(1, G.n - curr_size);
        candidates.reserve(estimated_candidates);
        
        for (int v = 0; v < G.n; ++v) {
            if (!inS[v]) {
                candidates.push_back(v);
                is_candidate[v] = 1;
            } else {
                is_candidate[v] = 0;
            }
        }
        
        // Mezclar candidatos para evitar sesgos
        shuffle(candidates.begin(), candidates.end(), rng);
    }

    // Inicialización greedy mejorada con múltiples criterios
    void init_greedy_maximal_improved() {
        int n = G.n;
        vector<int> order(n);
        iota(order.begin(), order.end(), 0);
        
        // Ordenar por múltiples criterios: grado, luego por ID para desempate
        sort(order.begin(), order.end(), [&](int a, int b) {
            if (G.degree[a] != G.degree[b]) 
                return G.degree[a] < G.degree[b];
            return a < b; // Desempate determinístico
        });

        // Aplicar greedy con mejora local
        for (int v : order) {
            if (!inS[v] && conf[v] == 0) {
                add_vertex(v);
                
                // Micro-optimización: intentar agregar vecinos de bajo grado
                vector<int> low_degree_neighbors;
                for (int u : G.adj[v]) {
                    if (!inS[u] && conf[u] == 0 && G.degree[u] <= 3) {
                        low_degree_neighbors.push_back(u);
                    }
                }
                
                for (int u : low_degree_neighbors) {
                    if (!inS[u] && conf[u] == 0) {
                        add_vertex(u);
                    }
                }
            }
        }
        
        update_candidates();
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

    // Versión local: completa a maximal solo en la región afectada
    void complete_to_maximal_local(const std::vector<int>& frontier_in) {
        std::deque<int> q;
        q.insert(q.end(), frontier_in.begin(), frontier_in.end());
        std::vector<char> inQueue(G.n, 0);
        for (int x : frontier_in)
            if (0 <= x && x < G.n)
                inQueue[x] = 1;

        while (!q.empty()) {
            int x = q.front();
            q.pop_front();
            inQueue[x] = 0;

            // Si x no está en S y no tiene vecinos en S, se puede agregar
            if (!inS[x] && conf[x] == 0) {
                add_vertex(x);
                // Al añadir x, sus vecinos podrían volverse candidatos
                for (int w : G.adj[x]) {
                    if (!inQueue[w]) {
                        q.push_back(w);
                        inQueue[w] = 1;
                    }
                }
            }
        }
    }

    // Registra mejor solución con estadísticas mejoradas
    void maybe_record_best(double elapsed_sec) {
        if (curr_size > best_size) {
            best_size = curr_size;
            best_inS = inS;
            best_time = elapsed_sec;
            last_improvement_time = elapsed_sec;
            improvements++;
            stagnation_count = 0;
            
            cout.setf(std::ios::fixed);
            cout << setprecision(6) << best_size << " " << best_time << "\n";
            cout.flush();
        }
    }

    // Estrategia de diversificación cuando hay estancamiento
    void diversify_solution() {
        if (!use_diversification) return;
        
        int original_size = curr_size;
        
        // Crear lista de vértices en S ordenados por prioridad
        vector<pair<int, int>> vertices_with_priority;
        vertices_with_priority.reserve(curr_size);
        
        for (int v = 0; v < G.n; ++v) {
            if (inS[v]) {
                vertices_with_priority.emplace_back(vertex_priority[v], v);
            }
        }
        
        // Ordenar por prioridad (más alta primero)
        sort(vertices_with_priority.begin(), vertices_with_priority.end(), 
             greater<pair<int, int>>());
        
        // Remover hasta 20% de los vértices con mayor prioridad
        int to_remove = min((int)vertices_with_priority.size(), 
                           max(1, curr_size / 5));
        
        for (int i = 0; i < to_remove; ++i) {
            int v = vertices_with_priority[i].second;
            remove_vertex(v);
        }
        
        // Completar a maximal
        complete_to_maximal();
        update_candidates();
        
        if (verbose) {
            cerr << "#diversify: removed=" << to_remove 
                 << " size " << original_size << "->" << curr_size << "\n";
        }
        
        // Reiniciar prioridades gradualmente (no completamente)
        for (int v = 0; v < G.n; ++v) {
            vertex_priority[v] = (vertex_priority[v] + G.degree[v]) / 2;
        }
    }

    // Control de temperatura adaptativo
    double adaptive_temperature_update(double T, double target_acceptance = 0.4) {
        if (!use_adaptive_temp) return T * 0.9995; // Comportamiento original
        
        const double tolerance = 0.05;
        const double fast_cooling = 0.985;
        const double normal_cooling = 0.9995;
        const double slow_cooling = 0.9999;
        
        if (acceptance_rate > target_acceptance + tolerance) {
            // Demasiadas aceptaciones -> enfriar más rápido
            return T * fast_cooling;
        } else if (acceptance_rate < target_acceptance - tolerance) {
            // Muy pocas aceptaciones -> enfriar más lento
            return T * slow_cooling;
        } else {
            // En el rango objetivo -> enfriamiento normal
            return T * normal_cooling;
        }
    }

    // Bucle principal optimizado con mejoras adaptativas
    void run(double tmax, double T0, double alpha, int iters_per_T) {
        Timer tim;
        init_greedy_maximal_improved();  // Inicialización mejorada
        
        // Ajuste dinámico más inteligente
        if (iters_per_T <= 0 || iters_per_T == 1000) {
            iters_per_T = max(1000, min(10000, G.n * 3));
        }

        maybe_record_best(0.0);

        double T = T0;
        if (G.n == 0) return;

        long long moves_in_block = 0;
        long long accepted_in_block = 0;

        while (true) {
            double el = tim.elapsed();
            if (el >= tmax) break;

            // Verificar estancamiento para diversificación
            if (use_diversification && el - last_improvement_time > tmax * 0.1) {
                stagnation_count++;
                if (stagnation_count >= 3) {
                    diversify_solution();
                    maybe_record_best(tim.elapsed());
                    stagnation_count = 0;
                    last_improvement_time = el;
                }
            }

            moves_in_block = 0;
            accepted_in_block = 0;

            // Bloque de iteraciones por temperatura
            for (int it = 0; it < iters_per_T; ++it) {
                if ((it & 511) == 0) {  // Chequeo más frecuente
                    el = tim.elapsed();
                    if (el >= tmax) break;
                }

                // Selección de vértice optimizada
                if (candidates.empty()) update_candidates();
                
                int v = -1;
                if (!candidates.empty() && U01(rng) < 0.8) {
                    // 80% del tiempo: usar candidatos precalculados
                    int idx = uniform_int_distribution<int>(0, (int)candidates.size()-1)(rng);
                    v = candidates[idx];
                    
                    // Optimización: remover candidato usado para evitar repetición inmediata
                    if (candidates.size() > 10) { // Solo si hay suficientes candidatos
                        swap(candidates[idx], candidates.back());
                        candidates.pop_back();
                    }
                } else {
                    // 20% del tiempo: selección completamente aleatoria
                    for (int tries = 0; tries < 16; ++tries) {
                        v = Uv(rng);
                        if (!inS[v]) break;
                    }
                    if (inS[v]) continue;
                }

                int k = conf[v];
                int delta = 1 - k;
                bool accept = (delta >= 0);
                
                moves_in_block++;
                total_moves++;

                if (!accept && T > 1e-9) {
                    double prob = exp(delta / T);
                    accept = (U01(rng) < prob);
                }

                if (accept) {
                    accepted_in_block++;
                    accepted_moves++;
                    
                    // Actualizar prioridad del vértice seleccionado
                    vertex_priority[v]++;
                    
                    // Aplicar movimiento
                    vector<int> removed;
                    for (int u : G.adj[v]) {
                        if (inS[u]) {
                            remove_vertex(u);
                            removed.push_back(u);
                            // Decrementar prioridad de vértices removidos
                            vertex_priority[u] = max(0, vertex_priority[u] - 1);
                        }
                    }

                    add_vertex(v);

                    // Construcción de frontera optimizada con estimación más precisa
                    int frontier_capacity = 1 + removed.size();
                    for (int u : removed) {
                        frontier_capacity += min(G.degree[u], G.n); // Limitar para evitar overflow
                    }
                    frontier_capacity += min(G.degree[v], G.n);
                    frontier_capacity = min(frontier_capacity, G.n * 2); // Limitar máximo
                    
                    vector<int> frontier;
                    frontier.reserve(frontier_capacity);
                    
                    frontier.push_back(v);
                    frontier.insert(frontier.end(), removed.begin(), removed.end());
                    for (int u : removed) {
                        frontier.insert(frontier.end(), G.adj[u].begin(), G.adj[u].end());
                    }
                    frontier.insert(frontier.end(), G.adj[v].begin(), G.adj[v].end());

                    complete_to_maximal_local(frontier);
                    maybe_record_best(tim.elapsed());
                    
                    // Actualizar candidatos si es necesario
                    if (it % 100 == 0) update_candidates();
                }
            }

            // Calcular tasa de aceptación para control adaptativo
            if (moves_in_block > 0) {
                acceptance_rate = 0.9 * acceptance_rate + 
                                0.1 * (double(accepted_in_block) / moves_in_block);
            }

            // Actualización de temperatura (adaptativa o fija)
            if (use_adaptive_temp) {
                T = adaptive_temperature_update(T);
            } else {
                T *= alpha;
            }
            
            if (T < 1e-12) T = 1e-12;
        }

        // Estadísticas finales (enviadas a stderr para no interferir con salida)
        if (total_moves > 0) {
            double final_acceptance_rate = double(accepted_moves) / total_moves;
            double avg_moves_per_second = total_moves / tim.elapsed();
            
            cerr << "#stats: total_moves=" << total_moves 
                 << " accepted=" << accepted_moves 
                 << " rate=" << fixed << setprecision(3) << final_acceptance_rate
                 << " improvements=" << improvements 
                 << " moves_per_sec=" << fixed << setprecision(0) << avg_moves_per_second
                 << "\n";
        }

        // Repetir mejor línea al final
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
// CLI mejorado con nuevas opciones
// ----------------------
struct Args {
    string   infile;
    double   tmax = 5.0;
    uint64_t seed = (uint64_t)chrono::high_resolution_clock::now().time_since_epoch().count();
    double   T0   = 2.0;        // Temperatura inicial más alta
    double   alpha= 0.9995;     // Enfriamiento más lento por defecto
    int      iters_per_T = 1000;
    bool     do_check = false;
    bool     diversify = false;    // Nueva: activar diversificación
    bool     adaptive_temp = false; // Nueva: control de temperatura adaptativo
    bool     verbose = false;      // Nueva: modo verbose para debugging
};

// Parseo mejorado con nuevas opciones
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
        else if (s=="--diversify") a.diversify = true;
        else if (s=="--adaptive_temp") a.adaptive_temp = true;
        else if (s=="--verbose" || s=="-v") a.verbose = true;
        else {
            cerr << "Unknown or incomplete arg: " << s << "\n";
            exit(1);
        }
    }
    if (a.infile.empty()) {
        cerr << "Usage: SA -i <graph_file> -t <seconds> [--seed S] [--T0 2.0] [--alpha 0.9995]\n"
             << "          [--iters_per_T N] [--check] [--diversify] [--adaptive_temp] [--verbose]\n";
        exit(1);
    }
    
    // Validaciones mejoradas
    if (a.tmax <= 0) a.tmax = 1.0;
    if (a.T0   <= 0) a.T0   = 2.0;
    if (a.alpha <= 0.5 || a.alpha >= 1.0) {
        cerr << "[WARN] alpha fuera de rango recomendado [0.5, 1.0); usando 0.9995\n";
        a.alpha = 0.9995;
    }
    if (a.iters_per_T <= 0) a.iters_per_T = 1000;
    
    return a;
}

int main(int argc, char** argv) {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    try {
        Args args = parse_args(argc, argv);
        Graph G = read_graph(args.infile);
        G.compute_degrees();  // Precalcular grados para optimización

        MIS_SA solver(G, args.seed);
        solver.use_diversification = args.diversify;
        solver.use_adaptive_temp = args.adaptive_temp;
        
        cerr << "#config: seed=" << args.seed 
             << " T0=" << args.T0 
             << " alpha=" << args.alpha
             << " diversify=" << (args.diversify ? "on" : "off")
             << " adaptive_temp=" << (args.adaptive_temp ? "on" : "off") 
             << " verbose=" << (args.verbose ? "on" : "off") << "\n";
             
        solver.verbose = args.verbose;
             
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
            } else {
                cerr << "[CHECK] OK: solución es independiente y maximal\n";
            }
        }
        return 0;
    } catch (const std::exception& e) {
        cerr << "ERROR: " << e.what() << "\n";
        return 1;
    }
}
