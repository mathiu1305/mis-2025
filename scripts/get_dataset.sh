#!/usr/bin/env bash
set -euo pipefail

# Script para descargar y descomprimir el dataset en ./data/
# Usa el enlace de Dropbox con dl=1 para descarga directa.

URL="https://www.dropbox.com/scl/fi/opxow3dx3jkdxukl4krwb/dataset_grafos_no_dirigidos.zip?rlkey=o5l953zrqfftd474aybxvd8jh&e=1&dl=1"
DEST="data"
ZIP="/tmp/dataset_grafos_no_dirigidos.zip"

mkdir -p "$DEST"

echo "[1/3] Descargando dataset (puede tardar)..."
curl -L "$URL" -o "$ZIP"

echo "[2/3] Descomprimiendo en $DEST/ ..."
unzip -q "$ZIP" -d "$DEST"

echo "[3/3] Limpiando temporales..."
rm -f "$ZIP"

echo "OK. Dataset listo en: $DEST/"
