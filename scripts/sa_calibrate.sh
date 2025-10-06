#!/usr/bin/env bash
# =============================================================================
# Script: sa_calibrate.sh
# -----------------------------------------------------------------------------
# Autores : Matías Gayoso, Constanza Obreque
# Entrega : 2 — Metaheurística de Trayectoria (Simulated Annealing)
# Descripción:
#   Ejecuta una grilla de parámetros para el binario SA sobre un conjunto de
#   instancias. Repite para varias semillas y guarda los resultados en CSV.
#
# Uso:
#   scripts/sa_calibrate.sh <bin_SA> <tiempo_seg> <instancias.txt> <seed_ini> <seed_fin> \
#       [--T0 "0.5 1.0 2.0"] [--alpha "0.995 0.997 0.999"]
#
# Ejemplo:
#   scripts/sa_calibrate.sh ./build/SA 5 scripts/instancias_muestra.txt 1 5 \
#       --T0 "0.5 1.0 2.0" --alpha "0.995 0.997 0.999"
#
# Salida:
#   results_sa_grid.csv  (columnas: instance,best_value,best_time,seconds,seed,T0,alpha)
# =============================================================================
set -euo pipefail

if [[ $# -lt 5 ]]; then
  echo "Uso: scripts/sa_calibrate.sh <bin_SA> <tiempo_seg> <instancias.txt> <seed_ini> <seed_fin> [--T0 \"...\"] [--alpha \"...\"]"
  exit 1
fi

BIN="$1"; TIME="$2"; INST_FILE="$3"; SEED_START="$4"; SEED_END="$5"; shift 5

# Valores por defecto de la grilla
T0_VALUES="1.0"
ALPHA_VALUES="0.999"

# Leer parámetros opcionales (--T0, --alpha)
while [[ $# -gt 0 ]]; do
  case "$1" in
    --T0)    T0_VALUES="$2"; shift 2 ;;
    --alpha) ALPHA_VALUES="$2"; shift 2 ;;
    *) echo "Aviso: argumento ignorado: $1"; shift ;;
  esac
done

# Validaciones mínimas
[[ -x "$BIN" ]] || { echo "ERROR: binario no ejecutable: $BIN"; exit 1; }
[[ -f "$INST_FILE" ]] || { echo "ERROR: archivo no encontrado: $INST_FILE"; exit 1; }

OUT="results_sa_grid.csv"
echo "instance,best_value,best_time,seconds,seed,T0,alpha" > "$OUT"

while IFS= read -r instance; do
  [[ -z "$instance" || "$instance" =~ ^# ]] && continue
  if [[ ! -f "$instance" ]]; then
    echo "Aviso: instancia no existe: $instance (se omite)"
    continue
  fi

  for T0 in $T0_VALUES; do
    for alpha in $ALPHA_VALUES; do
      for (( s=SEED_START; s<=SEED_END; s++ )); do
        echo "[RUN] inst=$instance  T0=$T0  alpha=$alpha  seed=$s  t=${TIME}s"
        # Tomamos la ÚLTIMA línea (any-time: la final repite la mejor)
        line=$("$BIN" -i "$instance" -t "$TIME" --T0 "$T0" --alpha "$alpha" --seed "$s" | tail -n 1)
        # reemplazar espacio por coma → "best_value,best_time"
        echo "$instance,$(echo "$line" | tr ' ' ','),$TIME,$s,$T0,$alpha" >> "$OUT"
      done
    done
  done
done < "$INST_FILE"

echo "Listo: $OUT"
