#!/usr/bin/env python3
import pandas as pd
import numpy as np

greedy_file = "results_greedy_all.csv"
prob_file   = "results_prob_a10_all.csv"
out_file    = "tabla_pauta.csv"

def clean_p(val):
    """
    Normaliza la densidad p a float, aceptando formatos raros:
    - '0.5', '0.55' -> float directo
    - '0c0.5', '0c0.55' -> '0.5', '0.55'
    - '0.0.5', '0.0.55' -> tomar la parte de la derecha como '0.5', '0.55'
    - cualquier cosa no parseable -> NaN
    """
    if isinstance(val, (int, float)):
        return float(val)
    s = str(val).strip()

    # 1) '0c0.5' -> '0.5'
    if 'c' in s:
        parts = s.split('c', 1)
        try:
            return float(parts[1])
        except:
            pass

    # 2) '0.0.5' -> usa la ultima parte como '0.5'
    if s.count('.') >= 2:
        # toma desde el penúltimo punto para la derecha: '0.0.55' -> '0.55'
        try:
            right = '.' + s.split('.')[-1]
            return float(right)
        except:
            pass

    # 3) normal: '0.5', '0.55'
    try:
        return float(s)
    except:
        return np.nan

# Cargar CSV
g = pd.read_csv(greedy_file).rename(columns=str.strip)
p = pd.read_csv(prob_file).rename(columns=str.strip)

# Limpiar y convertir p
g["p"] = g["p"].apply(clean_p)
p["p"] = p["p"].apply(clean_p)

# Filtra filas inválidas de p
g = g.dropna(subset=["p"])
p = p.dropna(subset=["p"])

# Mantener columnas necesarias del probabilista
p2 = p[["n","p","mean_value","mean_time"]].rename(
    columns={"mean_value":"prob_mean_value","mean_time":"prob_mean_time"}
)

# Unir por (n,p)
merged = pd.merge(g, p2, on=["n","p"], how="outer")

# Orden objetivo de p (0.1..0.9)
p_list = [round(x,1) for x in np.arange(0.1, 1.0, 0.1)]
n_list = sorted(merged["n"].dropna().astype(int).unique().tolist())

def fmt(v, d):
    if pd.isna(v): return ""
    return f"{float(v):.{d}f}"

rows = []
for n in n_list:
    # cabecera con N
    rows.append({
        "N": str(n),
        "densidad (p)": "",
        "Media Greedy": "",
        "promedio tiempo": "",
        "Media Greedy Aleatorio": "",
        "promedio tiempo ": "",
    })
    for pval in p_list:
        sel = merged[(merged["n"]==n) & (np.isclose(merged["p"], pval))]
        if len(sel) >= 1:
            r = sel.iloc[0]
            rows.append({
                "N": "",
                "densidad (p)": f"{pval:.1f}",
                "Media Greedy": fmt(r.get("mean_value"), 3),
                "promedio tiempo": fmt(r.get("mean_time"), 6),
                "Media Greedy Aleatorio": fmt(r.get("prob_mean_value"), 3),
                "promedio tiempo ": fmt(r.get("prob_mean_time"), 6),
            })
        else:
            rows.append({
                "N": "",
                "densidad (p)": f"{pval:.1f}",
                "Media Greedy": "",
                "promedio tiempo": "",
                "Media Greedy Aleatorio": "",
                "promedio tiempo ": "",
            })

out = pd.DataFrame(rows, columns=[
    "N",
    "densidad (p)",
    "Media Greedy",
    "promedio tiempo",
    "Media Greedy Aleatorio",
    "promedio tiempo "
])

out.to_csv(out_file, index=False)
print(f"✅ Generado {out_file}")
