#!/usr/bin/env bash
# Corre varias veces una misma instancia y calcula promedio de valor y tiempo
# Uso:
#   scripts/run_benchmark.sh <binario> <instancia.graph> <reps> [--alpha A]

set -euo pipefail

BIN="$1"; shift
FILE="$1"; shift
REPS="$1"; shift

if [[ ! -x "$BIN" ]]; then
  echo "No existe ejecutable: $BIN" >&2
  exit 1
fi
if [[ ! -f "$FILE" ]]; then
  echo "No existe archivo de entrada: $FILE" >&2
  exit 1
fi

SUMV=0
SUMT=0
for ((i=1;i<=REPS;i++)); do
  OUT=$("$BIN" -i "$FILE" "$@" --seed $i)
  V=$(awk '{print $1}' <<<"$OUT")
  T=$(awk '{print $2}' <<<"$OUT")
  SUMV=$(awk -v a="$SUMV" -v b="$V" 'BEGIN{print a+b}')
  SUMT=$(awk -v a="$SUMT" -v b="$T" 'BEGIN{print a+b}')
done

MEANV=$(awk -v s="$SUMV" -v n="$REPS" 'BEGIN{print s/n}')
MEANT=$(awk -v s="$SUMT" -v n="$REPS" 'BEGIN{print s/n}')
echo "$MEANV $MEANT"
