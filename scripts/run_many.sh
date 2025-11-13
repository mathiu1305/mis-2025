#!/usr/bin/env bash
# Uso: ./run_many.sh <sa_bin> <segundos> <lista_instancias> [runs=30]
set -euo pipefail
if [ $# -lt 3 ]; then
  echo "Uso: $0 <sa_bin> <segundos> <lista_instancias> [runs=30]" >&2; exit 1
fi
BIN=$1; TMAX=$2; LIST=$3; RUNS=${4:-30}
echo "instance,seconds,runs,mean_best,stdev_best,mean_time"
while IFS= read -r INST; do
  [ -z "$INST" ] && continue
  bash "$(dirname "$0")/run_batch.sh" "$BIN" "$INST" "$TMAX" "$RUNS" | tail -n 1
done < "$LIST"
