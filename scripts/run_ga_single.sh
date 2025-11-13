#!/usr/bin/env bash
set -euo pipefail

if [ "$#" -lt 2 ]; then
  echo "Uso: $0 instancia.graph tiempoSegundos [args extra para GA]"
  echo "Ejemplo:"
  echo "  $0 data/dataset_grafos_no_dirigidos/new_1000_dataset/erdos_n1000_p0c0.5_1.graph 10 --pop 100 --pc 0.8 --pm 0.05"
  exit 1
fi

INSTANCE="$1"
TIME_LIMIT="$2"
shift 2

# Ruta al binario (asumiendo make GA → build/GA)
GA_BIN="./build/GA"

if [ ! -x "$GA_BIN" ]; then
  echo "ERROR: No se encuentra el binario $GA_BIN o no es ejecutable."
  echo "Compila primero con: make GA"
  exit 1
fi

echo "Ejecutando GA:"
echo "  Instancia : $INSTANCE"
echo "  Tiempo    : $TIME_LIMIT s"
echo "  Extra args: $*"
echo

"$GA_BIN" -i "$INSTANCE" -t "$TIME_LIMIT" "$@"
