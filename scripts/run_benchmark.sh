#!/usr/bin/env bash
set -euo pipefail
BIN="$1"      # ./build/Greedy o ./build/Greedy-probabilista
INS="$2"      # ruta a instancia
RUNS="${3:-30}"  # por defecto 30
shift 3 || true

sumV=0
sumT=0
for r in $(seq 1 "$RUNS"); do
  # Para el probabilista conviene variar semilla:
  if [[ "$BIN" == *Greedy-probabilista ]]; then
    OUT=$(scripts/run_one.sh "$BIN" "$INS" "$@" --seed $RANDOM)
  else
    OUT=$(scripts/run_one.sh "$BIN" "$INS" "$@")
  fi
  V=$(echo "$OUT" | awk '{print $1}')
  T=$(echo "$OUT" | awk '{print $2}')
  # acumula en bash utilizando printf para evitar problemas de bc
  sumV=$(python3 - <<PY
print(float("$sumV")+float("$V"))
PY
)
  sumT=$(python3 - <<PY
print(float("$sumT")+float("$T"))
PY
)
done

avgV=$(python3 - <<PY
print(float("$sumV")/float("$RUNS"))
PY
)
avgT=$(python3 - <<PY
print(float("$sumT")/float("$RUNS"))
PY
)
echo "$avgV $avgT"
