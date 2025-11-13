// GA.cpp - Genetic Algorithm para MIS con any-time output (+ guardado de solución)
// Compilar: g++ -O3 -std=c++17 GA.cpp -o GA
#include <bits/stdc++.h>
using namespace std;

// ====== Utilidades de tiempo ======
struct Timer {
    chrono::steady_clock::time_point st;
    Timer() : st(chrono::steady_clock::now()) {}
    double elapsed() const {
        using namespace chrono;
        return duration_cast<duration<double>>(steady_clock::now() - st).count();
    }
};

// ====== Lector de grafos ======
// Soporta (a) DIMACS: "p edge n m" / "p edges n m" + "e u v" (1-indexed)
//         (b) Texto simple: primera línea "n m", luego m líneas "u v" (1-indexed)
struct Graph {
    int n = 0;
    vector<vector<int>> adj;

    static inline string trim(const string& s){
        size_t a=0,b=s.size();
        while(a<b && isspace((unsigned char)s[a])) ++a;
        while(b>a && isspace((unsigned char)s[b-1])) --b;
        return s.substr(a,b-a);
    }

    bool load(const string &path) {
        ifstream in(path);
        if (!in) return false;

        auto trim = [](string s)->string{
            size_t a=0,b=s.size();
            while(a<b && isspace((unsigned char)s[a])) ++a;
            while(b>a && isspace((unsigned char)s[b-1])) --b;
            return s.substr(a,b-a);
        };

        string line;
        vector<pair<long long,long long>> raw; // aristas leídas
        long long declared_n = -1;
        bool in_dimacs = false;
        bool saw_header_nm = false;  // "n m"
        bool saw_header_n  = false;  // "n" (solo número de nodos)
        long long max_id = LLONG_MIN;
        bool saw_zero = false;

        while (getline(in, line)) {
            line = trim(line);
            if (line.empty()) continue;
            if (line[0]=='c' || line[0]=='#') continue;

            // --- DIMACS header: "p edge n m" / "p edges n m"
            if (line[0]=='p') {
                in_dimacs = true;
                string p, name; long long nn=0, mm=0;
                stringstream ss(line);
                ss >> p >> name >> nn >> mm;
                if (p!="p") continue;
                if (nn>0) declared_n = nn;
                continue;
            }

            // --- Si venimos en DIMACS: líneas "e u v" o "a u v" o incluso "u v"
            if (in_dimacs) {
                {
                    stringstream ss(line);
                    char t; long long u,v;
                    if ((ss >> t >> u >> v) && (t=='e' || t=='a')) {
                        raw.emplace_back(u,v);
                        max_id = max(max_id, max(u,v));
                        if (u==0 || v==0) saw_zero = true;
                        continue;
                    }
                }
                {
                    stringstream ss(line);
                    long long u,v;
                    if ((ss >> u >> v)) {
                        raw.emplace_back(u,v);
                        max_id = max(max_id, max(u,v));
                        if (u==0 || v==0) saw_zero = true;
                        continue;
                    }
                }
                continue; // línea no válida en DIMACS -> ignorar
            }

            // --- No DIMACS: puede ser "n", "n m" o arista "u v"
            {
                stringstream ss(line);
                vector<long long> tok;
                long long x;
                while (ss >> x) tok.push_back(x);

                if (tok.size()==1 && declared_n<0 && !saw_header_n && !saw_header_nm) {
                    // Cabecera estilo: primera línea solo "n"
                    declared_n = tok[0];
                    saw_header_n = true;
                    continue;
                }

                if (tok.size()==2 && declared_n<0 && !saw_header_nm) {
                    // Cabecera estilo: primera línea "n m"
                    declared_n = tok[0];
                    saw_header_nm = true;
                    continue;
                }

                if (tok.size()>=2) {
                    long long u = tok[0], v = tok[1];
                    raw.emplace_back(u,v);
                    max_id = max(max_id, max(u,v));
                    if (u==0 || v==0) saw_zero = true;
                    continue;
                }
            }
            // Si llega acá, la línea no se reconoce: ignorar.
        }

        // Validaciones básicas
        if (declared_n < 0 && raw.empty()) return false;

        // 0-index si vimos algún 0
        bool zero_based = saw_zero;

        // Inferir n si no fue declarado
        if (declared_n < 0) {
            if (max_id == LLONG_MIN) return false;
            declared_n = zero_based ? (max_id + 1) : (max_id);
        }

        n = (int)declared_n;
        if (n <= 0) return false;

        adj.assign(n, {});
        for (auto [uu,vv] : raw) {
            long long u = uu, v = vv;
            if (!zero_based) { u -= 1; v -= 1; } // 1-based -> 0-based
            if (u<0 || v<0 || u>=n || v>=n || u==v) continue;
            adj[(int)u].push_back((int)v);
            adj[(int)v].push_back((int)u);
        }
        for (int i=0;i<n;i++) {
            auto &A = adj[i];
            sort(A.begin(), A.end());
            A.erase(unique(A.begin(), A.end()), A.end());
        }
        return true;
    }
};


// ====== Estado MIS (inS/conf + completar a maximal) ======
struct MISState {
    const Graph* G = nullptr;
    vector<uint8_t> inS;   // 0/1
    vector<int> conf;      // # vecinos dentro de S

    MISState(const Graph* g=nullptr){ reset(g); }
    void reset(const Graph* g){
        G = g;
        if (!g) return;
        inS.assign(g->n, 0);
        conf.assign(g->n, 0);
    }
    inline void add_vertex(int v){
        if (inS[v]) return;
        inS[v]=1;
        for(int u: G->adj[v]) conf[u]++;
    }
    inline void remove_vertex(int v){
        if (!inS[v]) return;
        inS[v]=0;
        for(int u: G->adj[v]) conf[u]--;
    }
    // completa a maximal local usando cola sobre frontera
    void complete_to_maximal_local(const vector<int> &frontier){
        deque<int> q(frontier.begin(), frontier.end());
        vector<uint8_t> inq(G->n,0);
        for(int x: q) if (x>=0 && x<G->n) inq[x]=1;

        while(!q.empty()){
            int x=q.front(); q.pop_front(); inq[x]=0;
            if (!inS[x] && conf[x]==0){
                add_vertex(x);
                for(int w: G->adj[x]) if(!inq[w]){ q.push_back(w); inq[w]=1; }
            }
        }
    }
    // completa globalmente
    void complete_global(){
        vector<int> frontier(G->n);
        iota(frontier.begin(), frontier.end(), 0);
        complete_to_maximal_local(frontier);
    }
    int size() const { return (int)count(inS.begin(), inS.end(), 1); }

    // reparación dirigida: quita vértices con conflictos priorizando mayor (conf, degree)
    void repair_and_complete(){
        vector<int> deg(G->n);
        for(int v=0; v<G->n; ++v) deg[v] = (int)G->adj[v].size();

        // Mientras exista conflicto, eliminar el vértice más "caro"
        int conflicted_cnt = 0;
        for(int v=0; v<G->n; ++v) if(inS[v] && conf[v]>0) conflicted_cnt++;

        while(conflicted_cnt>0){
            int worst = -1, wc=-1, wd=-1;
            for(int v=0; v<G->n; ++v){
                if(inS[v] && conf[v]>0){
                    int c = conf[v], d = deg[v];
                    if (c>wc || (c==wc && d>wd)){
                        wc=c; wd=d; worst=v;
                    }
                }
            }
            // quitar "worst" y actualizar contador
            if (worst==-1) break;
            remove_vertex(worst);
            // recomputar conflicted_cnt incrementalmente (barato)
            conflicted_cnt = 0;
            for(int v=0; v<G->n; ++v) if(inS[v] && conf[v]>0) conflicted_cnt++;
        }
        complete_global();
    }
};

// ====== Aleatorio ======
static thread_local std::mt19937 rng;
int rnd_int(int a, int b){ std::uniform_int_distribution<int> d(a,b); return d(rng); }
double rnd01(){ std::uniform_real_distribution<double> d(0.0,1.0); return d(rng); }

// ====== Individuo ======
struct Individual {
    vector<uint8_t> inS;
    int fit = 0;
};

// construye MIS factible por greedy determinista (grado ascendente)
MISState greedy_deterministic(const Graph& G){
    MISState st(&G);
    vector<int> order(G.n);
    iota(order.begin(), order.end(), 0);
    sort(order.begin(), order.end(), [&](int a,int b){ return G.adj[a].size() < G.adj[b].size(); });
    for(int v: order){
        if (st.conf[v]==0) st.add_vertex(v);
        // micro-completación de vecinos de grado bajo (<=3)
        for(int u: G.adj[v]) if ((int)G.adj[u].size()<=3 && st.conf[u]==0) st.add_vertex(u);
    }
    return st;
}

// greedy aleatorizado RCL-α (α en [0,1])
MISState greedy_alpha(const Graph& G, double alpha){
    MISState st(&G);
    vector<uint8_t> alive(G.n,1);
    vector<int> deg(G.n);
    for(int i=0;i<G.n;i++) deg[i]=G.adj[i].size();
    int vivos = G.n;

    while(vivos>0){
        int dmin=INT_MAX, dmax=-1;
        for(int u=0;u<G.n;u++) if(alive[u]){ dmin=min(dmin, deg[u]); dmax=max(dmax, deg[u]); }
        if (dmin==INT_MAX) break;
        double thr = dmin + alpha * (dmax - dmin);
        vector<int> RCL; RCL.reserve(G.n);
        int thr_int = (int)floor(thr+1e-9);
        for(int u=0;u<G.n;u++) if(alive[u] && deg[u] <= thr_int) RCL.push_back(u);
        if (RCL.empty()) { for(int u=0;u<G.n;u++) if(alive[u]) { RCL.push_back(u); break; } }

        int u = RCL[rnd_int(0,(int)RCL.size()-1)];
        if (st.conf[u]==0) st.add_vertex(u);

        // eliminar u y sus vecinos del conjunto "alive"
        vector<int> Elim = {u};
        for(int v: G.adj[u]) if(alive[v]) Elim.push_back(v);
        for(int r: Elim) if(alive[r]){
            alive[r]=0; --vivos;
            for(int w: G.adj[r]) if(alive[w]) deg[w]--;
        }
    }
    st.complete_global();
    return st;
}

// inicialización de población
struct GAParams {
    int pop = 80;
    double pc = 0.9;
    double pm = 0.03;
    int elitism = 2;
    int stall_gen = 100;
    string init = "mix";         // mix | greedy | random
    string repair = "frontier";  // placeholder
    bool ls = true;
};

struct CLI {
    string instPath;
    double tmax = -1;
    unsigned seed = 123;
    GAParams P;
    string out_sol = ""; // ruta opcional
};

Individual toIndividual(const MISState& st){
    Individual I; I.inS = st.inS; I.fit = (int)count(I.inS.begin(), I.inS.end(), 1); return I;
}

vector<Individual> init_population(const Graph& G, int pop, const string& init_mode){
    vector<Individual> P; P.reserve(pop);
    if (init_mode=="greedy" || init_mode=="mix"){
        auto gd = greedy_deterministic(G); P.push_back(toIndividual(gd));
    }
    while ((int)P.size() < pop){
        int mode = 0;
        if (init_mode=="mix") mode = rnd_int(0,1); // 0: alpha, 1: random
        else if (init_mode=="greedy") mode = 0;
        else mode = 1; // random

        MISState st(&G);
        if (mode==0){
            double a = 0.1 + 0.2*rnd01(); // alpha en [0.1,0.3]
            st = greedy_alpha(G, a);
        } else {
            // random factible + completar
            st.reset(&G);
            vector<int> perm(G.n); iota(perm.begin(), perm.end(), 0);
            shuffle(perm.begin(), perm.end(), rng);
            for(int v: perm) if (st.conf[v]==0 && rnd01()<0.5) st.add_vertex(v);
            st.complete_global();
        }
        P.push_back(toIndividual(st));
    }
    return P;
}

// torneo binario
int select_tournament(const vector<Individual>& P){
    int a = rnd_int(0,(int)P.size()-1);
    int b = rnd_int(0,(int)P.size()-1);
    return (P[a].fit >= P[b].fit)? a : b;
}

// cruce 1: unión + repair
Individual crossover_union_repair(const Graph& G, const Individual& A, const Individual& B){
    MISState st(&G);
    for(int v=0; v<G.n; ++v) if (A.inS[v] || B.inS[v]) st.add_vertex(v);
    st.repair_and_complete();
    return toIndividual(st);
}

// cruce 2: intersección + sesgo a grados bajos
Individual crossover_intersection_biased(const Graph& G, const Individual& A, const Individual& B){
    MISState st(&G);
    vector<int> frontier; frontier.reserve(G.n);

    for(int v=0; v<G.n; ++v){
        if (A.inS[v] && B.inS[v]) st.add_vertex(v);
    }
    vector<int> cand;
    cand.reserve(G.n);
    for(int v=0; v<G.n; ++v) if (A.inS[v]^B.inS[v]) cand.push_back(v);
    sort(cand.begin(), cand.end(), [&](int a,int b){ return G.adj[a].size() < G.adj[b].size(); });

    for(int v: cand){
        if (st.conf[v]==0) {
            st.add_vertex(v);
            for(int u: G.adj[v]) frontier.push_back(u);
        }
    }
    st.complete_to_maximal_local(frontier);
    return toIndividual(st);
}

// mutación suave
void mutate_soft(const Graph& G, Individual& I, double pm){
    if (pm <= 0.0) return;
    MISState st(&G); st.inS = I.inS; st.conf.assign(G.n,0);
    for(int v=0; v<G.n; ++v) if(st.inS[v]) for(int u: G.adj[v]) st.conf[u]++;

    int trials = max(1, (int)round(pm * G.n));
    while(trials--){
        int v = rnd_int(0, G.n-1);
        if (!st.inS[v] && st.conf[v]==0){
            st.add_vertex(v);
        } else if (st.inS[v] && rnd01()<0.2){
            st.remove_vertex(v);
        }
    }
    st.complete_global();
    I.inS = st.inS;
    I.fit = (int)count(I.inS.begin(), I.inS.end(), 1);
}

// mejora local 1-improvement
void local_search_1impr(const Graph& G, Individual& I){
    MISState st(&G); st.inS = I.inS; st.conf.assign(G.n,0);
    for(int v=0; v<G.n; ++v) if(st.inS[v]) for(int u: G.adj[v]) st.conf[u]++;
    vector<int> freeNodes; freeNodes.reserve(G.n);
    for(int v=0; v<G.n; ++v) if(!st.inS[v] && st.conf[v]==0) freeNodes.push_back(v);
    st.complete_to_maximal_local(freeNodes);
    I.inS = st.inS;
    I.fit = (int)count(I.inS.begin(), I.inS.end(), 1);
}

static inline void print_anytime(int best_fit, double best_time){
    cout << best_fit << " " << fixed << setprecision(6) << best_time << '\n' << flush;
}

static void write_solution_1indexed(const string& path, const vector<uint8_t>& inS){
    ofstream out(path);
    if(!out) return;
    bool first=true;
    for(size_t i=0;i<inS.size();++i){
        if (inS[i]){
            if(!first) out << " ";
            first=false;
            out << (i+1);
        }
    }
    out << "\n";
}

// ====== GA principal ======
int main(int argc, char** argv){
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    CLI C;

    // ---- parse CLI
    for (int i=1;i<argc;i++){
        string a = argv[i];
        auto need = [&](int &i){ if(i+1>=argc){ cerr<<"Falta valor para "<<a<<"\n"; exit(1);} };
        if (a=="-i"){ need(i); C.instPath = argv[++i]; }
        else if (a=="-t"){ need(i); C.tmax = atof(argv[++i]); }
        else if (a=="--seed"){ need(i); C.seed = (unsigned)stoul(argv[++i]); }
        else if (a=="--pop"){ need(i); C.P.pop = stoi(argv[++i]); }
        else if (a=="--pc"){ need(i); C.P.pc = atof(argv[++i]); }
        else if (a=="--pm"){ need(i); C.P.pm = atof(argv[++i]); }
        else if (a=="--elitism"){ need(i); C.P.elitism = stoi(argv[++i]); }
        else if (a=="--stall_gen"){ need(i); C.P.stall_gen = stoi(argv[++i]); }
        else if (a=="--init"){ need(i); C.P.init = argv[++i]; }
        else if (a=="--repair"){ need(i); C.P.repair = argv[++i]; } // reservado
        else if (a=="--ls"){ need(i); C.P.ls = (string(argv[++i])!="0"); }
        else if (a=="--out_sol"){ need(i); C.out_sol = argv[++i]; }
        else { /* ignorar desconocidos */ }
    }
    if (C.instPath.empty() || C.tmax <= 0.0){
        cerr << "Uso: ./GA -i instancia.graph -t segs "
             << "[--seed s --pop N --pc x --pm y --elitism k --stall_gen g "
             << "--init mix|greedy|random --repair frontier|greedydeg --ls 0|1 --out_sol ruta.txt]\n";
        return 1;
    }
    rng.seed(C.seed);

    Graph G;
    if (!G.load(C.instPath)){
        cerr << "No pude leer la instancia: " << C.instPath << "\n";
        return 2;
    }

    // Inicialización
    Timer timer;
    auto Pop = init_population(G, C.P.pop, C.P.init);

    int best_idx = 0;
    for(int i=1;i<(int)Pop.size();++i) if(Pop[i].fit > Pop[best_idx].fit) best_idx=i;
    int best_fit = Pop[best_idx].fit;
    double best_time = timer.elapsed();
    vector<uint8_t> best_sol = Pop[best_idx].inS;
    print_anytime(best_fit, best_time);

    int stall = 0;

    // Bucle evolutivo
    while (timer.elapsed() < C.tmax){
        // nueva población con elitismo
        vector<Individual> next; next.reserve(C.P.pop);
        vector<int> idx(Pop.size()); iota(idx.begin(), idx.end(), 0);
        sort(idx.begin(), idx.end(), [&](int a,int b){ return Pop[a].fit > Pop[b].fit; });
        for(int e=0;e<min(C.P.elitism,(int)Pop.size());++e) next.push_back(Pop[idx[e]]);

        // rellenar
        while ((int)next.size() < C.P.pop){
            const Individual& A = Pop[select_tournament(Pop)];
            const Individual& B = Pop[select_tournament(Pop)];
            Individual Cc;

            if (rnd01() < C.P.pc) {
                if (rnd01() < 0.5) Cc = crossover_union_repair(G, A, B);
                else               Cc = crossover_intersection_biased(G, A, B);
            } else {
                Cc = (A.fit >= B.fit) ? A : B; // copiar mejor padre
            }
            if (rnd01() < C.P.pm) mutate_soft(G, Cc, C.P.pm);
            if (C.P.ls) local_search_1impr(G, Cc);

            next.push_back(move(Cc));

            // any-time
            const Individual& last = next.back();
            if (last.fit > best_fit){
                best_fit = last.fit;
                best_time = timer.elapsed();
                best_sol  = last.inS;
                print_anytime(best_fit, best_time);
                stall = 0;
            }
        }
        Pop.swap(next);
        stall++;
        if (stall >= C.P.stall_gen) stall = 0;
        if (timer.elapsed() >= C.tmax) break;
    }

    // línea final + guardado opcional
    print_anytime(best_fit, best_time);
    if (!C.out_sol.empty()) write_solution_1indexed(C.out_sol, best_sol);
    return 0;
}
