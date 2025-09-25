#!/usr/bin/env bash
set -euo pipefail
# Recorre carpetas de instancias y genera promedios por (n,p) a CSV.
# Uso:
#   scripts/eval_folder.sh <bin_greedy> <bin_prob> <root_dir> <reps> <alpha> [--extra "..."]
# Ejemplo:
#   scripts/eval_folder.sh ./build/Greedy ./build/Greedy-probabilista data/dataset_grafos_no_dirigidos 30 0.1
#
# Salida:
#   results_greedy_all.csv, results_prob_aXX_all.csv en el directorio actual.

if [[ $# -lt 5 ]]; then
  echo "Uso: $0 <bin_greedy> <bin_prob> <root_dir> <reps> <alpha> [--extra \"args\"]" >&2
  exit 1
fi

BIN_G="$1"; BIN_P="$2"; ROOT="$3"; REPS="$4"; ALPHA="$5"; shift 5
EXTRA_ARGS=""
if [[ $# -ge 2 && "$1" == "--extra" ]]; then
  EXTRA_ARGS="$2"
fi

if [[ ! -x "$BIN_G" || ! -x "$BIN_P" ]]; then
  echo "Compila primero (make). No se encuentra $BIN_G o $BIN_P." >&2
  exit 1
fi
if [[ ! -d "$ROOT" ]]; then
  echo "No existe carpeta: $ROOT" >&2
  exit 1
fi

g_csv="results_greedy_all.csv"
p_csv=$(printf "results_prob_a%02d_all.csv" "$(awk -v a="$ALPHA" 'BEGIN{print int(a*100+0.5)}')")

echo "n,p,mean_value,mean_time" > "$g_csv"
echo "n,p,alpha,mean_value,mean_time" > "$p_csv"

shopt -s nullglob

# Recorre subcarpetas tipo new_1000_dataset, new_2000_dataset...
for dir in "$ROOT"/new_*_dataset; do
  [[ -d "$dir" ]] || continue
  base=$(basename "$dir")                          # new_1000_dataset
  n=$(sed -E 's/new_([0-9]+)_dataset/\1/' <<<"$base")

  # Detectar valores únicos de p por nombre de archivo: ..._p0cX.Y_*.graph
  declare -A seen_p=()
  for f in "$dir"/*.graph; do
    fname=$(basename "$f")
    if [[ "$fname" =~ _p([^_]+)_ ]]; then
      ptag="${BASH_REMATCH[1]}"        # "0c0.05"
      p_norm="${ptag//c/}"             # "00.05" -> 0.05 (equivalente numéricamente)
      seen_p["$p_norm"]=1
    fi
  done

  for p in "${!seen_p[@]}"; do
    files=( "$dir"/erdos_n${n}_p0c${p}_*.graph )
    [[ ${#files[@]} -gt 0 ]] || continue

    # Greedy determinista: promedio 1 corrida por archivo
    sum_v=0; sum_t=0; cnt=0
    for f in "${files[@]}"; do
      out=$("$BIN_G" -i "$f")
      v=$(awk '{print $1}' <<<"$out")
      t=$(awk '{print $2}' <<<"$out")
      sum_v=$(awk -v a="$sum_v" -v b="$v" 'BEGIN{print a+b}')
      sum_t=$(awk -v a="$sum_t" -v b="$t" 'BEGIN{print a+b}')
      cnt=$((cnt+1))
    done
    mean_v=$(awk -v s="$sum_v" -v n="$cnt" 'BEGIN{printf "%.10f", s/n}')
    mean_t=$(awk -v s="$sum_t" -v n="$cnt" 'BEGIN{printf "%.10f", s/n}')
    echo "$n,$p,$mean_v,$mean_t" >> "$g_csv"

    # Greedy probabilista: promedio de REPS por archivo, luego promedio entre archivos
    sum_v=0; sum_t=0; cnt=0
    for f in "${files[@]}"; do
      rep_sum_v=0; rep_sum_t=0
      for ((k=1;k<=REPS;k++)); do
        out=$("$BIN_P" -i "$f" --alpha "$ALPHA" --seed "$k" $EXTRA_ARGS)
        v=$(awk '{print $1}' <<<"$out")
        t=$(awk '{print $2}' <<<"$out")
        rep_sum_v=$(awk -v a="$rep_sum_v" -v b="$v" 'BEGIN{print a+b}')
        rep_sum_t=$(awk -v a="$rep_sum_t" -v b="$t" 'BEGIN{print a+b}')
      done
      rep_mean_v=$(awk -v s="$rep_sum_v" -v n="$REPS" 'BEGIN{printf "%.10f", s/n}')
      rep_mean_t=$(awk -v s="$rep_sum_t" -v n="$REPS" 'BEGIN{printf "%.10f", s/n}')
      sum_v=$(awk -v a="$sum_v" -v b="$rep_mean_v" 'BEGIN{print a+b}')
      sum_t=$(awk -v a="$sum_t" -v b="$rep_mean_t" 'BEGIN{print a+b}')
      cnt=$((cnt+1))
    done
    mean_v=$(awk -v s="$sum_v" -v n="$cnt" 'BEGIN{printf "%.10f", s/n}')
    mean_t=$(awk -v s="$sum_t" -v n="$cnt" 'BEGIN{printf "%.10f", s/n}')
    echo "$n,$p,$ALPHA,$mean_v,$mean_t" >> "$p_csv"
  done
done
