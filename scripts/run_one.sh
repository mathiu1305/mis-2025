#!/usr/bin/env bash
# Ejecuta una instancia con el algoritmo indicado y muestra valor + tiempo
# Uso:
#   scripts/run_one.sh <binario> <instancia.graph> [parametros_extra]

set -euo pipefail

BIN="$1"; shift
FILE="$1"; shift || true

if [[ ! -x "$BIN" ]]; then
  echo "No existe ejecutable: $BIN" >&2
  exit 1
fi
if [[ ! -f "$FILE" ]]; then
  echo "No existe archivo de entrada: $FILE" >&2
  exit 1
fi

"$BIN" -i "$FILE" "$@"
