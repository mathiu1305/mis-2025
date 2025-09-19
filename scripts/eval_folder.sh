#!/usr/bin/env bash
# Uso:
#   scripts/eval_folder.sh <bin_greedy> <bin_prob> <root_dataset_dir> <reps> <alpha> [--extra "..."]
# Ejemplo:
#   scripts/eval_folder.sh ./build/Greedy ./build/Greedy-probabilista data/dataset_grafos_no_dirigidos 30 0.1
#
# Salida:
#   results_greedy_all.csv, results_prob_aXX_all.csv en el directorio actual.
#

# Produce CSV con columnas:
# n,p,algo,alpha,repeats,files,mean_value,mean_time

set -euo pipefail

if [[ $# -lt 2 ]]; then
  echo "Uso: $0 <dir_dataset> <greedy|prob> [--alpha A] [--repeats R]" >&2
  exit 1
fi

DIR="$1"; shift
ALGO="$1"; shift

ALPHA=""
REPEATS=1

while [[ $# -gt 0 ]]; do
  case "$1" in
    --alpha)   ALPHA="$2"; shift 2;;
    --repeats) REPEATS="$2"; shift 2;;
    *) echo "Arg no reconocido: $1" >&2; exit 1;;
  esac
done

BIN_GREEDY="./build/Greedy"
BIN_PROB="./build/Greedy-probabilista"

if [[ ! -x "$BIN_GREEDY" || ! -x "$BIN_PROB" ]]; then
  echo "Compila primero (make). No se encuentra build/Greedy o build/Greedy-probabilista." >&2
  exit 1
fi

if [[ ! -d "$DIR" ]]; then
  echo "No existe carpeta: $DIR" >&2
  exit 1
fi

# Encuentra etiquetas de p únicas desde el nombre de archivo: ..._p<PTAG>_*.graph
# Ejemplo de PTAG: "0c0.05" -> normalizamos quitando 'c' -> "00.05" (que numéricamente es 0.05)
mapfile -t PTAGS < <(ls "$DIR"/*.graph 2>/dev/null | sed -E 's/.*_p([^_]+)_.*/\1/' | sort -u)
if [[ ${#PTAGS[@]} -eq 0 ]]; then
  echo "No se encontraron .graph en $DIR" >&2
  exit 1
fi

# CSV header
echo "n,p,algo,alpha,repeats,files,mean_value,mean_time"

for PTAG in "${PTAGS[@]}"; do
  FILES=( "$DIR"/*"_p${PTAG}_"*.graph )
  # n desde la 1ra línea del primer archivo del grupo
  if [[ ! -f "${FILES[0]}" ]]; then continue; fi
  N=$(head -n1 "${FILES[0]}")

  # normaliza la etiqueta p
  P_NORM=$(echo "$PTAG" | sed 's/c//g')  # "0c0.05" -> "00.05" (equivale a 0.05)

  SUMV=0
  SUMT=0
  COUNT_RUNS=0
  COUNT_FILES=0

  for F in "${FILES[@]}"; do
    [[ -f "$F" ]] || continue
    COUNT_FILES=$((COUNT_FILES+1))

    if [[ "$ALGO" == "greedy" ]]; then
      # determinista: 1 corrida por archivo
      OUT=$("$BIN_GREEDY" -i "$F")
      V=$(echo "$OUT" | awk '{print $1}')
      T=$(echo "$OUT" | awk '{print $2}')
      SUMV=$(python3 - <<PY
print(float("$SUMV")+float("$V"))
PY
)
      SUMT=$(python3 - <<PY
print(float("$SUMT")+float("$T"))
PY
)
      COUNT_RUNS=$((COUNT_RUNS+1))

    elif [[ "$ALGO" == "prob" ]]; then
      # aleatorizado: REPEATS corridas por archivo (semilla distinta)
      for r in $(seq 1 "$REPEATS"); do
        OUT=$("$BIN_PROB" -i "$F" --alpha "${ALPHA:-0.3}" --seed $RANDOM)
        V=$(echo "$OUT" | awk '{print $1}')
        T=$(echo "$OUT" | awk '{print $2}')
        SUMV=$(python3 - <<PY
print(float("$SUMV")+float("$V"))
PY
)
        SUMT=$(python3 - <<PY
print(float("$SUMT")+float("$T"))
PY
)
        COUNT_RUNS=$((COUNT_RUNS+1))
      done
    else
      echo "ALGO debe ser 'greedy' o 'prob'." >&2; exit 1
    fi
  done

  MEANV=$(python3 - <<PY
print(float("$SUMV")/float("$COUNT_RUNS"))
PY
)
  MEANT=$(python3 - <<PY
print(float("$SUMT")/float("$COUNT_RUNS"))
PY
)

  # alpha vacío -> mostrar 0 si no aplica
  ASHOW="${ALPHA:-0}"
  echo "$N,$P_NORM,$ALGO,$ASHOW,$REPEATS,$COUNT_FILES,$MEANV,$MEANT"
done
