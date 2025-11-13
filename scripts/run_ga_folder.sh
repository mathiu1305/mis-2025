#!/usr/bin/env bash
set -euo pipefail

if [ "$#" -lt 2 ]; then
  echo "Uso: $0 carpeta_con_graph tiempoSegundos [args extra para GA]"
  echo "Ejemplo:"
  echo "  $0 data/dataset_grafos_no_dirigidos/new_1000_dataset 10 --pop 100 --pc 0.8 --pm 0.05"
  exit 1
fi

FOLDER="$1"
TIME_LIMIT="$2"
shift 2

GA_BIN="./build/GA"

if [ ! -x "$GA_BIN" ]; then
  echo "ERROR: No se encuentra el binario $GA_BIN o no es ejecutable."
  echo "Compila primero con: make GA"
  exit 1
fi

if [ ! -d "$FOLDER" ]; then
  echo "ERROR: La carpeta $FOLDER no existe."
  exit 1
fi

for INSTANCE in "$FOLDER"/*.graph; do
  [ -e "$INSTANCE" ] || continue
  echo "======================================================"
  echo "Instancia: $INSTANCE"
  echo "Tiempo   : $TIME_LIMIT s"
  echo "======================================================"
  "$GA_BIN" -i "$INSTANCE" -t "$TIME_LIMIT" "$@"
  echo
done
