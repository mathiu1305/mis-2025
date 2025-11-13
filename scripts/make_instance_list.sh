#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
OUT="${1:-$ROOT/Instances/n1000.list}"
DIR="${2:-$ROOT/data/dataset_grafos_no_dirigidos/new_1000_dataset}"
find "$DIR" -maxdepth 1 -type f -name "*.graph" | sort > "$OUT"
echo "OK -> $OUT ($(wc -l < "$OUT") archivos)"
