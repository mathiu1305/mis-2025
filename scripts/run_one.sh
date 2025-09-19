#!/usr/bin/env bash
set -euo pipefail
BIN="$1"     # ej: ./build/Greedy o ./build/Greedy-probabilista
INS="$2"     # ruta a instancia
shift 2
OUT=$("$BIN" -i "$INS" "$@")
# El bin ya imprime: "<valor> <tiempo>"
echo "$OUT"
