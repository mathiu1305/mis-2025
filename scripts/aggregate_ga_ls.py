import pandas as pd
import re

# Cargar CSV GA+LS
df = pd.read_csv("ga_ls_final.csv")

# Función para extraer n y p desde el nombre del archivo
def parse_instance(path):
    fname = path.split("/")[-1]
    match = re.search(r"n(\d+)_p0c([0-9.]+)", fname)
    if not match:
        raise ValueError(f"No se pudo parsear instancia: {fname}")

    n = int(match.group(1))
    p_raw = float(match.group(2))   # ej: 0.05, 0.10, 0.15, ..., 0.90

    # Mapeo a tu tabla (0.1, 0.2, ..., 0.9):
    # 0.05 y 0.10 -> 0.10
    # 0.15 y 0.20 -> 0.20
    # ...
    p_tabla = min(0.9, (int(p_raw * 10 + 0.999999)) / 10.0)
    
    return n, p_tabla

# Extraer n y p
df[["n", "p"]] = df["instance"].apply(
    lambda x: pd.Series(parse_instance(x))
)

# Agregar estadísticas por (n, p)
agg = (
    df
    .groupby(["n", "p"])
    .agg(
        GA_LS_med=("mis_size", "mean"),
        GA_LS_dev=("mis_size", "std"),
        GA_LS_t=("solve_time", "mean")
    )
    .reset_index()
)

# Redondeos (consistentes con LaTeX)
agg["GA_LS_med"] = agg["GA_LS_med"].round(3)
agg["GA_LS_dev"] = agg["GA_LS_dev"].round(3)
agg["GA_LS_t"]   = agg["GA_LS_t"].round(6)

# Ordenar
agg = agg.sort_values(["n", "p"])

# Guardar resultado
agg.to_csv("ga_ls_aggregated.csv", index=False)

print("✔ ga_ls_aggregated.csv generado correctamente")
print(agg.head(10))
