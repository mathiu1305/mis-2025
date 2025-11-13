#!/bin/bash

# Corre GA en una mini-muestra de 9 instancias (N=1000,2000,3000 x p=0.1,0.5,0.9)
# y genera resultados en 10s y 60s con la config de irace.

# Detectar binario GA
if [ -x ./build/GA ]; then
  GA=./build/GA
elif [ -x ./src/ga/GA ]; then
  GA=./src/ga/GA
else
  echo "ERROR: No se encontró el binario GA (build/GA o src/ga/GA)." >&2
  exit 1
fi

mkdir -p results

OUT10="results/ga_sample_10s.csv"
OUT60="results/ga_sample_60s.csv"

echo "instance,N,p,best_size,time" > "$OUT10"
echo "instance,N,p,best_size,time" > "$OUT60"

INSTANCES=(
"data/dataset_grafos_no_dirigidos/new_1000_dataset/erdos_n1000_p0c0.1_1.graph"
"data/dataset_grafos_no_dirigidos/new_1000_dataset/erdos_n1000_p0c0.5_1.graph"
"data/dataset_grafos_no_dirigidos/new_1000_dataset/erdos_n1000_p0c0.9_1.graph"

"data/dataset_grafos_no_dirigidos/new_2000_dataset/erdos_n2000_p0c0.1_1.graph"
"data/dataset_grafos_no_dirigidos/new_2000_dataset/erdos_n2000_p0c0.5_1.graph"
"data/dataset_grafos_no_dirigidos/new_2000_dataset/erdos_n2000_p0c0.9_1.graph"

"data/dataset_grafos_no_dirigidos/new_3000_dataset/erdos_n3000_p0c0.1_1.graph"
"data/dataset_grafos_no_dirigidos/new_3000_dataset/erdos_n3000_p0c0.5_1.graph"
"data/dataset_grafos_no_dirigidos/new_3000_dataset/erdos_n3000_p0c0.9_1.graph"
)

for F in "${INSTANCES[@]}"; do
  if [ ! -f "$F" ]; then
    echo "ADVERTENCIA: instancia no encontrada: $F" >&2
    continue
  fi

  NAME=$(basename "$F" .graph)
  N=$(echo "$NAME" | cut -d_ -f2 | sed 's/n//')
  P=$(echo "$NAME" | cut -d_ -f3 | sed 's/p0c/0./')

  echo ">>> Procesando $NAME (N=$N, p=$P)..."

  # 10 segundos
  LINE10=$("$GA" \
    -i "$F" -t 10 \
    --pop 129 --pc 0.9955 --pm 0.1512 \
    --elitism 1 --stall_gen 101 --init mix --ls 1 \
    --seed 123 | tail -n 1)

  BEST10=$(echo "$LINE10" | awk '{print $1}')
  TIME10=$(echo "$LINE10" | awk '{print $2}')
  echo "$NAME,$N,$P,$BEST10,$TIME10" >> "$OUT10"

  # 60 segundos
  LINE60=$("$GA" \
    -i "$F" -t 60 \
    --pop 129 --pc 0.9955 --pm 0.1512 \
    --elitism 1 --stall_gen 101 --init mix --ls 1 \
    --seed 123 | tail -n 1)

  BEST60=$(echo "$LINE60" | awk '{print $1}')
  TIME60=$(echo "$LINE60" | awk '{print $2}')
  echo "$NAME,$N,$P,$BEST60,$TIME60" >> "$OUT60"

done

echo "Listo. Resultados en:"
echo "  $OUT10"
echo "  $OUT60"
