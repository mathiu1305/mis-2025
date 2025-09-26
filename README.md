# MIS 2025 — Heurísticas Greedy para Maximum Independent Set (MIS)

**Autores:** Matias Gayoso, Constanza Obreque  
**Entorno objetivo:** Windows + WSL2 (Ubuntu) + VSCode  
**Lenguaje:** C++17

---

## 0) Requisitos previos

En WSL / Ubuntu, instala las dependencias mínimas:

```bash
sudo apt update
sudo apt -y install build-essential curl unzip
```

---

## 1) Clonar el repositorio

**SSH (recomendado):**
```bash
git clone git@github.com:mathiu1305/mis-2025.git
cd mis-2025
```

**HTTPS (alternativa):**
```bash
git clone https://github.com/mathiu1305/mis-2025.git
cd mis-2025
```

⚠️ Importante en WSL: trabaja siempre dentro de `/home/<usuario>/...` (Linux).  
Evita clonar/compilar desde rutas de Windows (`\\wsl.localhost\...`) por temas de permisos y rendimiento.

---

## 2) Dataset

El proyecto usa el dataset **`dataset_grafos_no_dirigidos`** (Erdős–Rényi).

### Opción A — Descarga automática (recomendada)

```bash
chmod +x scripts/get_dataset.sh
scripts/get_dataset.sh
```

Esto descarga y descomprime el ZIP en `data/dataset_grafos_no_dirigidos/`.

### Opción B — Descarga manual

1. Descargar:
   ```bash
   curl -L "https://www.dropbox.com/scl/fi/opxow3dx3jkdxukl4krwb/dataset_grafos_no_dirigidos.zip?rlkey=o5l953zrqfftd474aybxvd8jh&e=1&dl=1" -o /tmp/dataset.zip
   ```
2. Descomprimir en la carpeta `data/`:
   ```bash
   mkdir -p data
   unzip -q /tmp/dataset.zip -d data/
   ```
3. Verifica estructura:
   ```
   mis-2025/
   ├─ data/
   │  └─ dataset_grafos_no_dirigidos/
   │     ├─ new_1000_dataset/
   │     ├─ new_2000_dataset/
   │     └─ new_3000_dataset/
   ```

---

## 3) Compilación

Desde la raíz del proyecto:

```bash
make
```

Esto genera:
- `build/Greedy`
- `build/Greedy-probabilista`

Si quieres recompilar limpio:
```bash
make clean && make
```

---

## 4) Ejecución básica

Los programas imprimen en **una sola línea**:
```
<valor_objetivo> <tiempo_segundos>
```

Ejemplo:
```bash
./build/Greedy -i data/dataset_grafos_no_dirigidos/new_1000_dataset/erdos_n1000_p0c0.05_1.graph
./build/Greedy-probabilista -i data/dataset_grafos_no_dirigidos/new_1000_dataset/erdos_n1000_p0c0.05_1.graph --alpha 0.1 --seed 1
```

Salida típica:
```
99 0.000354
104 0.000527
```

---

## 5) Scripts de evaluación

### `scripts/get_dataset.sh`
Descarga y descomprime automáticamente el dataset en la carpeta `data/`.  
Útil para preparar el entorno con un solo comando.

### `scripts/run_one.sh`
Ejecuta un algoritmo (Greedy o Greedy-probabilista) sobre **una sola instancia**.  
Imprime el resultado de esa corrida (valor y tiempo).

Ejemplo:
```bash
scripts/run_one.sh ./build/Greedy data/dataset_grafos_no_dirigidos/new_1000_dataset/erdos_n1000_p0c0.05_1.graph
```

### `scripts/run_benchmark.sh`
Ejecuta un algoritmo sobre la **misma instancia** varias veces (ej: 30 repeticiones) y devuelve el promedio de tamaño de solución y tiempo.

Ejemplo:
```bash
scripts/run_benchmark.sh ./build/Greedy-probabilista data/dataset_grafos_no_dirigidos/new_1000_dataset/erdos_n1000_p0c0.05_1.graph 30 --alpha 0.1
# => "<media_valor> <media_tiempo>"
```

### `scripts/eval_folder.sh`
Ejecuta ambos algoritmos (`Greedy` y `Greedy-probabilista`) en todas las instancias de un directorio.  
Genera automáticamente dos archivos CSV:
- `results_greedy_all.csv`
- `results_prob_a10_all.csv`

Cada línea contiene los resultados promedio por configuración `(n,p)`.

Ejemplo:
```bash
scripts/eval_folder.sh ./build/Greedy ./build/Greedy-probabilista data/dataset_grafos_no_dirigidos 30 0.1
```

---

## 6) Estructura del proyecto

```
mis-2025/
├─ src/                 # código fuente en C++
├─ include/             # headers extra
├─ scripts/             # scripts auxiliares
│  ├─ get_dataset.sh       # descarga dataset
│  ├─ run_one.sh           # ejecuta una instancia
│  ├─ run_benchmark.sh     # ejecuta N veces una instancia y promedia
│  └─ eval_folder.sh       # ejecuta sobre carpetas completas y genera CSVs
├─ build/               # binarios compilados
├─ data/                # dataset descargado aquí
│  └─ dataset_grafos_no_dirigidos/
├─ results_*.csv        # resultados (no versionados en Git)
├─ Makefile
└─ README.md
```

---

## 7) Problemas frecuentes (FAQ)

- **`make: g++: command not found`** → Instala dependencias: `sudo apt -y install build-essential`.
- **Permisos en scripts** → `chmod +x scripts/*.sh`.
- **Ruta dataset incorrecta** → Verifica que los `.graph` estén bajo `data/dataset_grafos_no_dirigidos/...`.
- **Lento en WSL** → Evita trabajar en rutas montadas de Windows (`/mnt/c/...`). Usa `/home/<usuario>/`.
