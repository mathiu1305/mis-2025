#!/usr/bin/env python3
# =============================================================================
# Script : aggregate_sa_results.py
# -----------------------------------------------------------------------------
# Autores : Matías Gayoso, Constanza Obreque
# Entrega : 2 — Metaheurística de Trayectoria (Simulated Annealing)
# Descripción:
#   Lee el CSV generado por sa_calibrate.sh y produce un resumen agregado por
#   (n, p, T0, alpha, seconds): media, desviación, min, max y tiempo a mejor.
#
# Uso:
#   python3 scripts/aggregate_sa_results.py results_sa_grid.csv
#
# Salida:
#   results_sa_grid_agg.csv
#   Columnas: n, p, T0, alpha, seconds, count, mean_best, std_best, min_best,
#             max_best, mean_time_to_best, std_time_to_best
# =============================================================================
import sys
import pandas as pd

if len(sys.argv) < 2:
    print("Uso: python3 scripts/aggregate_sa_results.py results_sa_grid.csv")
    sys.exit(1)

inp = sys.argv[1]
df = pd.read_csv(inp)

# Se espera: instance,best_value,best_time,seconds,seed,T0,alpha

def parse_n_p(path: str):
    """
    Intenta extraer n y p del nombre del archivo con patrón:
    erdos_n<NNNN>_p0c<PPPP>_*.graph, p=0.cPPPP
    Ej: 'erdos_n1000_p0c0.05_1.graph' -> (1000, 0.05)
    """
    try:
        base = path.split("/")[-1]
        parts = base.split("_")
        n = int(parts[1][1:])                    # 'n1000' -> 1000
        p = float(parts[2][1:].replace("c", "."))# 'p0c0.05' -> 0.05
        return n, p
    except Exception:
        return None, None

df["n"], df["p"] = zip(*df["instance"].map(parse_n_p))

# Agregación por hiperparámetros y tiempo
agg = df.groupby(["n", "p", "T0", "alpha", "seconds"]).agg(
    count=("best_value", "count"),
    mean_best=("best_value", "mean"),
    std_best=("best_value", "std"),
    min_best=("best_value", "min"),
    max_best=("best_value", "max"),
    mean_time_to_best=("best_time", "mean"),
    std_time_to_best=("best_time", "std"),
).reset_index()

out = "results_sa_grid_agg.csv"
agg.to_csv(out, index=False)
print(f"OK -> {out}")
