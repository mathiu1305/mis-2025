#!/usr/bin/env bash
# Ejecuta GA en la mini-muestra, con la configuración encontrada por irace,
# para t=10s y t=60s. Los resultados se guardan en results/, NO se suben al repo.
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT_DIR"

CONFIG="--pop 129 --pc 0.9955 --pm 0.1512 --elitism 1 --stall_gen 101 --init mix --ls 1"
SEED=123

mkdir -p results

echo "Instancia,N,p,t,seed,best_size" > results/ga_mini_10s.csv
while read -r line; do
  [[ -z "$line" || "$line" =~ ^# ]] && continue
  inst="$line"

  # Extraer N y p desde el nombre (simple parsing)
  fname="$(basename "$inst")"
  N="$(echo "$fname" | sed -E 's/.*_n([0-9]+)_p0c0\.([0-9]+)_.*/\1/')"
  P="0.$(echo "$fname" | sed -E 's/.*_n[0-9]+_p0c0\.([0-9]+)_.*/\1/')"

  echo "Ejecutando 10s en $inst ..."
  best_line=$(./bin/GA -i "$inst" -t 10 --seed "$SEED" $CONFIG | tail -n 1)
  best_size=$(echo "$best_line" | awk '{print $1}')
  echo "$inst,$N,$P,10,$SEED,$best_size" >> results/ga_mini_10s.csv
done < tuning/ga/instances-list-mini-10s.txt

echo "Instancia,N,p,t,seed,best_size" > results/ga_mini_60s.csv
while read -r line; do
  [[ -z "$line" || "$line" =~ ^# ]] && continue
  inst="$line"

  fname="$(basename "$inst")"
  N="$(echo "$fname" | sed -E 's/.*_n([0-9]+)_p0c0\.([0-9]+)_.*/\1/')"
  P="0.$(echo "$fname" | sed -E 's/.*_n[0-9]+_p0c0\.([0-9]+)_.*/\1/')"

  echo "Ejecutando 60s en $inst ..."
  best_line=$(./bin/GA -i "$inst" -t 60 --seed "$SEED" $CONFIG | tail -n 1)
  best_size=$(echo "$best_line" | awk '{print $1}')
  echo "$inst,$N,$P,60,$SEED,$best_size" >> results/ga_mini_60s.csv
done < tuning/ga/instances-list-mini-60s.txt

echo "Listo. Revisa results/ga_mini_10s.csv y results/ga_mini_60s.csv (no subir a git)."

