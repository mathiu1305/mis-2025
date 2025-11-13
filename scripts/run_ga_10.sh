#!/bin/bash

# Wrapper para ejecutar GA con t=10s usando la config de irace

# Detectar binario GA
if [ -x ./build/GA ]; then
  GA=./build/GA
elif [ -x ./src/ga/GA ]; then
  GA=./src/ga/GA
else
  echo "ERROR: No se encontró el binario GA (build/GA o src/ga/GA)." >&2
  exit 1
fi

if [ $# -lt 1 ]; then
  echo "Uso: $0 instancia.graph [seed]" >&2
  exit 1
fi

INSTANCE="$1"
SEED="${2:-123}"

"$GA" \
  -i "$INSTANCE" \
  -t 10 \
  --pop 129 --pc 0.9955 --pm 0.1512 \
  --elitism 1 --stall_gen 101 --init mix --ls 1 \
  --seed "$SEED"
