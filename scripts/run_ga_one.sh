#!/usr/bin/env bash
set -euo pipefail

# Uso:
#   ./scripts/run_ga_one.sh <instancia.graph> [BUDGETS] [SEEDS] [OUT_CSV]
# Ej:
#   ./scripts/run_ga_one.sh data/.../erdos_n1000_p0c0.35_1.graph 10 1 results/ga_one.csv
#   ./scripts/run_ga_one.sh data/.../erdos_n1000_p0c0.35_1.graph "10,30" "1,2" results/ga_one.csv

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BIN="$ROOT/build/GA"
INST="${1:?Falta instancia .graph}"
BUDGETS="${2:-10}"
SEEDS="${3:-1}"
OUT_CSV="${4:-$ROOT/results/ga_one.csv}"

POP="${POP:-80}"
PC="${PC:-0.9}"
PM="${PM:-0.03}"
ELIT="${ELIT:-2}"
STALL="${STALL:-100}"
INIT="${INIT:-mix}"
REPAIR="${REPAIR:-frontier}"
LS="${LS:-1}"

mkdir -p "$ROOT/results" "$ROOT/results/sols"

IFS=',' read -r -a BUD_ARR <<< "$BUDGETS"
IFS=',' read -r -a SEED_ARR <<< "$SEEDS"

# encabezado csv si no existe
if [[ ! -f "$OUT_CSV" ]]; then
  echo "instance,seed,budget_sec,pop,pc,pm,elitism,stall_gen,init,repair,ls,best_size,best_time_sec,sol_path" > "$OUT_CSV"
fi

base="$(basename "$INST" .graph)"
for b in "${BUD_ARR[@]}"; do
  for s in "${SEED_ARR[@]}"; do
    SOL="$ROOT/results/sols/${base}_t${b}_s${s}.txt"
    line="$("$BIN" -i "$INST" -t "$b" --seed "$s" \
                  --pop "$POP" --pc "$PC" --pm "$PM" \
                  --elitism "$ELIT" --stall_gen "$STALL" \
                  --init "$INIT" --repair "$REPAIR" --ls "$LS" \
                  --out_sol "$SOL" \
            | tail -n1 || true)"
    if [[ -z "$line" ]]; then
      best_size="NA"; best_time="NA"
    else
      best_size="$(awk '{print $1}' <<< "$line")"
      best_time="$(awk '{print $2}' <<< "$line")"
    fi
    echo "$INST,$s,$b,$POP,$PC,$PM,$ELIT,$STALL,$INIT,$REPAIR,$LS,$best_size,$best_time,$SOL" >> "$OUT_CSV"
    printf "✔ %s | t=%-3s | seed=%s | best=%s @ %ss | sol=%s\n" "$base" "$b" "$s" "$best_size" "$best_time" "$SOL"
  done
done
