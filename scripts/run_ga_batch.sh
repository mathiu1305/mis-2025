#!/usr/bin/env bash
set -euo pipefail

# === Config por defecto (puedes sobreescribir via args o variables) ===
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BIN="$ROOT/build/GA"
DATASET_DIR="${1:-$ROOT/data/dataset_grafos_no_dirigidos/new_1000_dataset}"
OUT_CSV="${2:-$ROOT/results/ga_anytime_all.csv}"

# Parámetros GA (tú puedes tunearlos luego)
POP="${POP:-80}"
PC="${PC:-0.9}"
PM="${PM:-0.03}"
ELIT="${ELIT:-2}"
STALL="${STALL:-100}"
INIT="${INIT:-mix}"
REPAIR="${REPAIR:-frontier}"
LS="${LS:-1}"

# Presupuestos (segundos) y semillas a usar
BUDGETS="${BUDGETS:-10,60}"   # 10s y 60s
SEEDS="${SEEDS:-1,2,3}"       # 3 semillas

# === Helpers ===
IFS=',' read -r -a BUD_ARR <<< "$BUDGETS"
IFS=',' read -r -a SEED_ARR <<< "$SEEDS"

# Encabezado CSV si no existe
if [[ ! -f "$OUT_CSV" ]]; then
  echo "instance,n_guess,seed,pop,pc,pm,elitism,stall_gen,init,repair,ls,best_size,best_time_sec,budget_sec" > "$OUT_CSV"
fi

# Función para inferir n desde el nombre (best-effort)
infer_n() {
  local fname="$1"
  # intenta capturar _nXXXX_ de "erdos_n1000_..."
  if [[ "$fname" =~ _n([0-9]+)_ ]]; then
    echo "${BASH_REMATCH[1]}"
  else
    # fallback vacío
    echo ""
  fi
}

# === Loop principal ===
shopt -s nullglob
found=0
for inst in "$DATASET_DIR"/*.graph; do
  found=1
  base="$(basename "$inst")"
  n_guess="$(infer_n "$base")"

  for b in "${BUD_ARR[@]}"; do
    for s in "${SEED_ARR[@]}"; do
      # Ejecuta GA y toma la última línea (any-time final)
      line="$("$BIN" -i "$inst" -t "$b" --seed "$s" \
                   --pop "$POP" --pc "$PC" --pm "$PM" \
                   --elitism "$ELIT" --stall_gen "$STALL" \
                   --init "$INIT" --repair "$REPAIR" --ls "$LS" \
             | tail -n1 || true)"

      # Si por algún motivo no hubo línea, escribe NA
      if [[ -z "$line" ]]; then
        best_size="NA"; best_time="NA"
      else
        best_size="$(awk '{print $1}' <<< "$line")"
        best_time="$(awk '{print $2}' <<< "$line")"
      fi

      echo "${inst},${n_guess},${s},${POP},${PC},${PM},${ELIT},${STALL},${INIT},${REPAIR},${LS},${best_size},${best_time},${b}" \
        >> "$OUT_CSV"

      # Feedback en consola
      printf "✔ %s | t=%-3s | seed=%s | best=%s @ %ss\n" "$base" "$b" "$s" "$best_size" "$best_time"
    done
  done
done

if [[ "$found" -eq 0 ]]; then
  echo "No se encontraron archivos .graph en: $DATASET_DIR" >&2
  exit 2
fi
