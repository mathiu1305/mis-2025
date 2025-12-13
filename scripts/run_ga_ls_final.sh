#!/bin/bash
set -euo pipefail

# ===============================
# Benchmark final GA + LS (tuned)
# ===============================

INSTANCES_FILE="tuning/ga/instances-list.txt"
BIN="./build/solver_MISP"

POP=79
LS_BUDGET=444
LS_FREQ=5
TIME_LIMIT=10      # segundos (ajusta si necesitas 5s)
SEED=1

OUTDIR="results/final_ga_ls"
OUTCSV="$OUTDIR/ga_ls_final.csv"

mkdir -p "$OUTDIR"

# Header CSV
echo "instance,seed,time_limit,pop,ls_budget,ls_freq,mis_size,solve_time" > "$OUTCSV"

COUNT=0
TOTAL=$(wc -l < "$INSTANCES_FILE")

while read -r INSTANCE; do
  COUNT=$((COUNT+1))
  echo "[GA+LS] ($COUNT/$TOTAL) $INSTANCE"

  RESULT=$(
    $BIN \
      -i "$INSTANCE" \
      -t "$TIME_LIMIT" \
      --seed "$SEED" \
      --pop "$POP" \
      --ls_budget "$LS_BUDGET" \
      --ls_freq "$LS_FREQ" \
      2>/dev/null | tail -n 1
  )

  MIS_SIZE=$(echo "$RESULT" | awk '{print $1}')
  SOLVE_TIME=$(echo "$RESULT" | awk '{print $2}')

  # Fallback por seguridad
  MIS_SIZE=${MIS_SIZE:-0}
  SOLVE_TIME=${SOLVE_TIME:-0}

  echo "$INSTANCE,$SEED,$TIME_LIMIT,$POP,$LS_BUDGET,$LS_FREQ,$MIS_SIZE,$SOLVE_TIME" >> "$OUTCSV"

done < "$INSTANCES_FILE"

echo "✅ Benchmark GA+LS terminado"
echo "Resultados en: $OUTCSV"

