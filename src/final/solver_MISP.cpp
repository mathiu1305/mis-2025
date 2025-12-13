// solver_MISP.cpp
// Híbrido MH_p + MH_t para Maximum Independent Set
// GA + Búsqueda Local (LS)
// CLI: solver_MISP -i instancia -t tiempo [params]
// Any-time output: "<best> <time>"

#include <bits/stdc++.h>
#include "../utils.hpp"

using namespace std;

// =======================
// Lector robusto (DIMACS / n m / pares)
// Normaliza a 0-based y deduplica
// =======================
struct GraphR {
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
        if(!in) return false;

        string line;
        vector<pair<long long,long long>> raw;
        long long declared_n = -1;
        bool in_dimacs=false;
        bool saw_header_nm=false, saw_header_n=false;
        long long max_id = LLONG_MIN;
        bool saw_zero=false;

        while(getline(in,line)){
            line=trim(line);
            if(line.empty()) continue;
            if(line[0]=='c' || line[0]=='#') continue;

            if(line[0]=='p'){
                in_dimacs=true;
                string p,name; long long nn=0,mm=0;
                stringstream ss(line);
                ss>>p>>name>>nn>>mm;
                if(nn>0) declared_n=nn;
                continue;
            }

            if(in_dimacs){
                { // e u v
                    stringstream ss(line);
                    char t; long long u,v;
                    if((ss>>t>>u>>v) && (t=='e' || t=='a')){
                        raw.emplace_back(u,v);
                        max_id=max(max_id,max(u,v));
                        if(u==0||v==0) saw_zero=true;
                        continue;
                    }
                }
                { // u v
                    stringstream ss(line);
                    long long u,v;
                    if((ss>>u>>v)){
                        raw.emplace_back(u,v);
                        max_id=max(max_id,max(u,v));
                        if(u==0||v==0) saw_zero=true;
                        continue;
                    }
                }
                continue;
            }

            // no DIMACS
            {
                stringstream ss(line);
                vector<long long> tok;
                long long x;
                while(ss>>x) tok.push_back(x);

                if(tok.size()==1 && declared_n<0 && !saw_header_n && !saw_header_nm){
                    declared_n=tok[0];
                    saw_header_n=true;
                    continue;
                }
                if(tok.size()==2 && declared_n<0 && !saw_header_nm){
                    declared_n=tok[0];
                    saw_header_nm=true;
                    continue;
                }
                if(tok.size()>=2){
                    long long u=tok[0], v=tok[1];
                    raw.emplace_back(u,v);
                    max_id=max(max_id,max(u,v));
                    if(u==0||v==0) saw_zero=true;
                    continue;
                }
            }
        }

        if(declared_n<0 && raw.empty()) return false;
        bool zero_based = saw_zero;

        if(declared_n<0){
            if(max_id==LLONG_MIN) return false;
            declared_n = zero_based ? (max_id+1) : (max_id);
        }

        n=(int)declared_n;
        if(n<=0) return false;

        adj.assign(n,{});
        for(auto [uu,vv]: raw){
            long long u=uu, v=vv;
            if(!zero_based){ u-=1; v-=1; }
            if(u<0||v<0||u>=n||v>=n||u==v) continue;
            adj[(int)u].push_back((int)v);
            adj[(int)v].push_back((int)u);
        }
        for(int i=0;i<n;i++){
            auto &A=adj[i];
            sort(A.begin(),A.end());
            A.erase(unique(A.begin(),A.end()),A.end());
        }
        return true;
    }
};


// =======================
// Estado MIS reutilizable
// =======================
struct MISState {
    const GraphR* G;
    vector<char> inS;
    vector<int> conf;
    int size;

    MISState(const GraphR* g=nullptr) { reset(g); }

    void reset(const GraphR* g){
        G = g;
        inS.assign(G->n, 0);
        conf.assign(G->n, 0);
        size = 0;
    }

    void add(int v){
        inS[v] = 1;
        size++;
        for(int u : G->adj[v]) conf[u]++;
    }

    void remove(int v){
        inS[v] = 0;
        size--;
        for(int u : G->adj[v]) conf[u]--;
    }

    // Reparación + completar a maximal
    void repair_and_complete(){
        vector<int> deg(G->n);
        for(int i=0;i<G->n;i++) deg[i] = G->adj[i].size();

        bool changed = true;
        while(changed){
            changed = false;
            for(int v=0;v<G->n;v++){
                if(inS[v] && conf[v]>0){
                    remove(v);
                    changed = true;
                }
            }
        }
        for(int v=0;v<G->n;v++){
            if(!inS[v] && conf[v]==0) add(v);
        }
    }
};

// =======================
// Greedy simple (init)
// =======================
MISState greedy_init(const GraphR& G){
    MISState S(&G);
    vector<int> order(G.n);
    iota(order.begin(), order.end(), 0);
    sort(order.begin(), order.end(),
         [&](int a,int b){ return G.adj[a].size() < G.adj[b].size(); });

    for(int v : order){
        if(S.conf[v]==0) S.add(v);
    }
    return S;
}

// =======================
// Búsqueda Local (MH_t)
// 1-exchange simple
// =======================
void local_search(MISState& S, int budget){
    int n = S.G->n;
    for(int it=0; it<budget; it++){
        int v = rand() % n;
        if(S.inS[v]) continue;

        bool ok = true;
        for(int u : S.G->adj[v]){
            if(S.inS[u]){ ok=false; break; }
        }
        if(ok){
            S.add(v);
        }
    }
}


// =======================
// Selección por torneo
// =======================
int tournament(const vector<MISState>& pop){
    int a = rand() % pop.size();
    int b = rand() % pop.size();
    return (pop[a].size > pop[b].size) ? a : b;
}

// =======================
// Cruce uniforme
// =======================
MISState crossover(const MISState& A, const MISState& B){
    MISState C(A.G);
    for(int v=0; v<A.G->n; v++){
        if(rand() & 1){
            if(A.inS[v] && C.conf[v]==0) C.add(v);
        }else{
            if(B.inS[v] && C.conf[v]==0) C.add(v);
        }
    }
    C.repair_and_complete();
    return C;
}

// =======================
// MAIN
// =======================
int main(int argc, char** argv){
    string instance;
    double Tlimit = 10.0;
    int pop_size = 40;
    int ls_budget = 200;
    int seed = 1;
    int ls_k = 4;
    int ls_freq = 10;
    
    for(int i=1;i<argc;i++){
        string a = argv[i];
        if(a=="-i") instance = argv[++i];
        else if(a=="-t") Tlimit = stod(argv[++i]);
        else if(a=="--pop") pop_size = stoi(argv[++i]);
        else if(a=="--ls_budget") ls_budget = stoi(argv[++i]);
        else if(a=="--seed") seed = stoi(argv[++i]);
	else if(a=="--ls_k") ls_k = stoi(argv[++i]);
	else if(a=="--ls_freq") ls_freq = stoi(argv[++i]);
    }

    srand(seed);

    GraphR G;
	if(!G.load(instance)){
    		cerr << "ERROR: No se pudo leer grafo: " << instance << "\n";
   	 return 1;
	}

    double t0 = now_seconds();

    // =======================
    // Inicialización población
    // =======================
    vector<MISState> pop;
    for(int i=0;i<pop_size;i++){
        pop.push_back(greedy_init(G));
    }

    MISState best = pop[0];
    double best_time = 0.0;

    cout << best.size << " " << 0.0 << endl;

    // =======================
    // Loop principal GA
    // =======================
    while(true){
        double t = now_seconds() - t0;
        if(t >= Tlimit) break;

        // Selección de padres
	int p1 = tournament(pop);
	int p2 = tournament(pop);

	// Cruce
	MISState child = crossover(pop[p1], pop[p2]);

	// Mutación simple
	int v = rand() % G.n;
	if(child.inS[v]) child.remove(v);
	else if(child.conf[v]==0) child.add(v);

	child.repair_and_complete();


        // Intensificación solo para top-k
	static long long iter = 0;
iter++;

// aplicar LS cada ls_freq iteraciones
bool apply_ls = (ls_budget > 0) && (ls_freq > 0) && (iter % ls_freq == 0);

if(apply_ls){
    // Umbral: estimar "top-k" sin ordenar: tomamos una muestra
    int samples = min(pop_size, 10);
    int thr = 0;
    for(int s=0;s<samples;s++){
        int idx = rand()%pop_size;
        thr = max(thr, pop[idx].size);
    }
    // Si el hijo es al menos tan bueno como el mejor de una muestra,
    // lo intensificamos (aprox top-k, barato)
    if(child.size >= thr){
        local_search(child, ls_budget);
        child.repair_and_complete();
    }
}


        if(child.size > best.size){
            best = child;
            best_time = now_seconds() - t0;
            cout << best.size << " " << best_time << endl;
        }

        // Reemplazo
        int r = rand()%pop_size;
        pop[r] = child;
    }

    // Salida final
    cout << best.size << " " << best_time << endl;
    return 0;
}
