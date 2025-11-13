
# Runner para irace: ejecuta GA en una instancia con ciertos parámetros
set -euo pipefail

if [ "$#" -lt 2 ]; then
  echo "Uso: $0 instancia seed [parametros...]" >&2
  exit 1
fi

INSTANCE="$1"
SEED="$2"
shift 2

# Parámetros extra pasados por irace, por ejemplo:
# --pop 129 --pc 0.9955 --pm 0.1512 --elitism 1 --stall_gen 101 --init mix --ls 1
PARAMS="$@"

# Asegurarse de estar en la raíz del proyecto (execDir = . en scenario.txt)
# (Si irace ya se lanza en la raíz, esto no hace nada)
PROJ_ROOT="$(dirname "$0")/../.."
cd "$PROJ_ROOT"

# Ejecutar GA con límite de 10s (tuning se hace a 10s)
# GA debe imprimir líneas "<size> <time>"; tomamos el último size.
OUT=$(
  ./bin/GA \
    -i "$INSTANCE" \
    -t 10 \
    --seed "$SEED" \
    $PARAMS \
    2>/dev/null | tail -n 1 | awk '{print $1}'
)

# Por seguridad, si no hay salida, devolvemos algo muy malo
if [ -z "$OUT" ]; then
  OUT=0
fi
echo "$OUT"

