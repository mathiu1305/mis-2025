#!/usr/bin/env bash
# Uso: ./run_batch.sh <sa_bin> <instancia> <segundos> [runs=30]
set -euo pipefail
if [ $# -lt 3 ]; then
  echo "Uso: $0 <sa_bin> <instancia> <segundos> [runs=30]" >&2; exit 1
fi
BIN=$1; INST=$2; TMAX=$3; RUNS=${4:-30}
BESTS=$(mktemp); TIMES=$(mktemp); trap 'rm -f "$BESTS" "$TIMES"' EXIT
for SEED in $(seq 1 "$RUNS"); do
  LINE=$("$BIN" -i "$INST" -t "$TMAX" --seed "$SEED" | tail -n 1)
  BEST=$(awk '{print $1}' <<<"$LINE"); TIME=$(awk '{print $2}' <<<"$LINE")
  echo "$BEST" >> "$BESTS"; echo "$TIME" >> "$TIMES"
done
python3 - "$INST" "$TMAX" "$RUNS" "$BESTS" "$TIMES" <<'PY'
import sys, statistics as st
inst, tmax, runs, bf, tf = sys.argv[1:6]
best  = [float(x) for x in open(bf) if x.strip()]
times = [float(x) for x in open(tf) if x.strip()]
def stats(xs):
    m = sum(xs)/len(xs) if xs else float('nan')
    s = 0.0 if len(xs)<=1 else st.stdev(xs)
    return m, s
mb, sb = stats(best); mt, _ = stats(times)
print("instance,seconds,runs,mean_best,stdev_best,mean_time")
print(f"{inst},{tmax},{runs},{mb:.6f},{sb:.6f},{mt:.6f}")
PY
