#!/bin/bash
# Runner para irace: ejecuta solver_MISP (MH_p + MH_t)
set -euo pipefail

if [ "$#" -lt 2 ]; then
  echo "Uso: $0 instancia seed [parametros...]" >&2
  exit 1
fi

INSTANCE="$1"
SEED="$2"
shift 2

PARAMS="$@"

PROJ_ROOT="$(dirname "$0")/../.."
cd "$PROJ_ROOT"

OUT=$(
  ./build/solver_MISP \
    -i "$INSTANCE" \
    -t 10 \
    --seed "$SEED" \
    $PARAMS \
    2>/dev/null | tail -n 1 | awk '{print $1}'
)

if [ -z "$OUT" ]; then
  OUT=0
fi

echo "$OUT"
