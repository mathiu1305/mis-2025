#!/usr/bin/env bash
set -euo pipefail

# 1) Dependencias mínimas (idempotente)
sudo apt-get update -y
sudo apt-get install -y build-essential curl unzip

# 2) Dataset si falta
if [ ! -d "data/dataset_grafos_no_dirigidos" ]; then
  echo "[SMOKE] Descargando dataset..."
  chmod +x scripts/get_dataset.sh
  scripts/get_dataset.sh
fi

# 3) Compilar
echo "[SMOKE] Compilando..."
make

# 4) Elegir una instancia chica y correr
G="data/dataset_grafos_no_dirigidos/new_1000_dataset/erdos_n1000_p0c0.05_18.graph"
echo "[SMOKE] Ejecutando Greedy..."
./build/Greedy -i "$G"
echo "[SMOKE] Ejecutando Greedy-probabilista..."
./build/Greedy-probabilista -i "$G" --alpha 0.1 --seed 1

echo "[SMOKE] OK"
