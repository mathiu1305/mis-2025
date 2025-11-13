#!/usr/bin/env bash
set -euo pipefail

GA_BIN="./build/GA"

if [ ! -x "$GA_BIN" ]; then
  echo "ERROR: No se encuentra el binario $GA_BIN o no es ejecutable."
  echo "Compila primero con: make GA"
  exit 1
fi

RESULTS_DIR="results"
mkdir -p "$RESULTS_DIR"

OUT10="$RESULTS_DIR/ga_10s_mini.csv"
OUT60="$RESULTS_DIR/ga_60s_mini.csv"

# Configuración óptima encontrada por irace
IRACE_ARGS=(--pop 129 --pc 0.9955 --pm 0.1512 --elitism 1 --stall_gen 101 --init mix --ls 1)

# Mini-muestra de instancias (paths relativos desde la raíz del repo)
INSTANCES=(
  "data/dataset_grafos_no_dirigidos/new_1000_dataset/erdos_n1000_p0c0.1_1.graph"
  "data/dataset_grafos_no_dirigidos/new_1000_dataset/erdos_n1000_p0c0.5_1.graph"
  "data/dataset_grafos_no_dirigidos/new_1000_dataset/erdos_n1000_p0c0.9_1.graph"

  "data/dataset_grafos_no_dirigidos/new_2000_dataset/erdos_n2000_p0c0.1_1.graph"
  "data/dataset_grafos_no_dirigidos/new_2000_dataset/erdos_n2000_p0c0.5_1.graph"
  "data/dataset_grafos_no_dirigidos/new_2000_dataset/erdos_n2000_p0c0.9_1.graph"

  "data/dataset_grafos_no_dirigidos/new_3000_dataset/erdos_n3000_p0c0.1_1.graph"
  "data/dataset_grafos_no_dirigidos/new_3000_dataset/erdos_n3000_p0c0.5_1.graph"
  "data/dataset_grafos_no_dirigidos/new_3000_dataset/erdos_n3000_p0c0.9_1.graph"
)

echo "Generando:"
echo "  $OUT10"
echo "  $OUT60"
echo

# Cabeceras de los CSV
echo "N,p,time_limit,best_size,best_time,instance" > "$OUT10"
echo "N,p,time_limit,best_size,best_time,instance" > "$OUT60"

run_set () {
  local TIME_LIMIT="$1"
  local OUT_FILE="$2"

  for INST in "${INSTANCES[@]}"; do
    if [ ! -f "$INST" ]; then
      echo "ADVERTENCIA: no se encuentra la instancia $INST, se omite."
      continue
    fi

    # Extraer N y p desde el nombre del archivo
    # Ejemplo: erdos_n1000_p0c0.5_1.graph
    local BASENAME
    BASENAME="$(basename "$INST")"

    # N: parte después de 'n' hasta '_'
    local NVAL
    NVAL="$(echo "$BASENAME" | sed -E 's/.*n([0-9]+)_p.*/\1/')"

    # p: tomar p0c0.5 → 0.5
    local PVAL
    PVAL="$(echo "$BASENAME" | sed -E 's/.*p0c0\.([0-9]+).*/0.\1/')"

    echo "======================================================"
    echo "Instancia : $INST"
    echo "N         : $NVAL"
    echo "p         : $PVAL"
    echo "Tiempo    : $TIME_LIMIT s"
    echo "Config    : ${IRACE_ARGS[*]}"
    echo "======================================================"

    # Ejecutar GA y capturar la última línea (any-time)
    # Formato esperado: "<best_size> <time>"
    LAST_LINE="$("$GA_BIN" -i "$INST" -t "$TIME_LIMIT" "${IRACE_ARGS[@]}" | tail -n 1 || true)"

    if [ -z "$LAST_LINE" ]; then
      echo "ADVERTENCIA: ejecución sin salida para $INST (time_limit=$TIME_LIMIT)"
      continue
    fi

    BEST_SIZE="$(echo "$LAST_LINE" | awk '{print $1}')"
    BEST_TIME="$(echo "$LAST_LINE" | awk '{print $2}')"

    echo "  → best_size = $BEST_SIZE, best_time = $BEST_TIME"

    # Añadir fila al CSV
    echo "${NVAL},${PVAL},${TIME_LIMIT},${BEST_SIZE},${BEST_TIME},${INST}" >> "$OUT_FILE"
    echo
  done
}

echo "=== Ejecutando GA con 10s en mini-muestra ==="
run_set 10 "$OUT10"

echo "=== Ejecutando GA con 60s en mini-muestra ==="
run_set 60 "$OUT60"

echo
echo "Listo."
echo "  Resultados 10s → $OUT10"
echo "  Resultados 60s → $OUT60"
