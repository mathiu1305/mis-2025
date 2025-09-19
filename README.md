# MIS 2025 — Heurísticas Greedy para Maximum Independent Set (MIS)

**Autores:** Matias Gayoso, Contanza Obreque  
**Entorno objetivo:** Windows + WSL2 (Ubuntu) + VSCode  
**Lenguaje:** C++17

---

## 0) Requisitos

### En WSL / Ubuntu
```bash
sudo apt update
sudo apt -y install build-essential curl unzip
```

> Opcional (para análisis o scripts): `python3`, `pip`, etc.

---

## 1) Clonado del repositorio

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

> **Importante WSL:** trabaja siempre dentro de `/home/<usuario>/...` (Linux). Evita operar el repo desde rutas de Windows (`\\wsl.localhost\...`) para no tener problemas de permisos/rendimiento.

---

## 2) Dataset

El proyecto usa el dataset **`dataset_grafos_no_dirigidos`** (grafos no dirigidos Erdős–Rényi), distribuido como **ZIP**.

- **URL (Dropbox)**:  
  `https://www.dropbox.com/scl/fi/opxow3dx3jkdxukl4krwb/dataset_grafos_no_dirigidos.zip?rlkey=o5l953zrqfftd474aybxvd8jh&e=1&dl=0`

### Opción A — Descarga automática (script recomendado)

Ejecuta el script que proveemos (descarga y descomprime en `data/`):
```bash
# desde la raíz del repo
chmod +x scripts/get_dataset.sh
scripts/get_dataset.sh
```

### Opción B — Descarga manual

1. Descarga con `curl -L` (forzando seguimiento de redirecciones) y `dl=1`:
   ```bash
   curl -L "https://www.dropbox.com/scl/fi/opxow3dx3jkdxukl4krwb/dataset_grafos_no_dirigidos.zip?rlkey=o5l953zrqfftd474aybxvd8jh&e=1&dl=1" -o /tmp/dataset_grafos_no_dirigidos.zip
   ```
2. Descomprime en la carpeta `data/` del repo:
   ```bash
   mkdir -p data
   unzip -q /tmp/dataset_grafos_no_dirigidos.zip -d data/
   ```
3. Verifica estructura (ejemplo):
   ```
   mis-2025/
   ├─ data/
   │  └─ dataset_grafos_no_dirigidos/
   │     ├─ new_1000_dataset/
   │     │  ├─ erdos_n1000_p0c0.05_1.graph
   │     │  ├─ erdos_n1000_p0c0.05_2.graph
   │     │  └─ ...
   │     ├─ new_2000_dataset/
   │     └─ new_3000_dataset/
   └─ ...
   ```

> Cada archivo `.graph` comienza con `n` (número de nodos) en la primera línea y luego pares `u v` (0-based) por línea indicando aristas.

---

## 3) Compilación

Desde la raíz del repositorio:
```bash
make
```

Se generan los binarios en `build/`:
- `build/Greedy`
- `build/Greedy-probabilista`

---

## 4) Ejecución mínima (pauta)

Salida estándar **en una sola línea**:
```
<valor_objetivo> <tiempo_segundos>
```

### Comandos
```bash
./build/Greedy -i data/dataset_grafos_no_dirigidos/new_1000_dataset/erdos_n1000_p0c0.05_1.graph

./build/Greedy-probabilista -i data/dataset_grafos_no_dirigidos/new_1000_dataset/erdos_n1000_p0c0.05_1.graph --alpha 0.1 --seed 1
```

**Ejemplo de salida:**
```
100 0.000396
```
(= valor objetivo 100, tiempo 0.000396 s)

---

## 5) Heurísticas incluidas (Entrega 1)

- **Greedy determinista**  
  Selecciona iterativamente el vértice de **menor grado actual**, lo añade al MIS y elimina a sus vecinos del grafo residual.

- **Greedy aleatorizado (RCL-α)**  
  Construye una lista restringida de candidatos con umbral:  
  d_min + α (d_max - d_min)  
  Selecciona al azar desde la RCL. `--alpha` controla el nivel de aleatoriedad y `--seed` fija la semilla.

---

## 6) Scripts de ayuda

- **Una corrida** (muestra `<valor> <tiempo>`):
  ```bash
  scripts/run_one.sh ./build/Greedy data/dataset_grafos_no_dirigidos/new_1000_dataset/erdos_n1000_p0c0.05_1.graph
  ```

- **N corridas (promedio)**:
  ```bash
  scripts/run_benchmark.sh ./build/Greedy-probabilista data/dataset_grafos_no_dirigidos/new_1000_dataset/erdos_n1000_p0c0.05_1.graph 30 --alpha 0.1
  # => imprime "<media_valor> <media_tiempo>"
  ```

- **Evaluar carpetas completas** (promedia por configuración `(n,p)` y exporta CSV):
  ```bash
  scripts/eval_folder.sh ./build/Greedy ./build/Greedy-probabilista data/dataset_grafos_no_dirigidos 30 0.1
  # -> results_greedy_all.csv  y  results_prob_a10_all.csv
  ```

> Si algún script no corre por permisos, usa: `chmod +x scripts/*.sh`.

---

## 7) Estructura del proyecto

```
mis-2025/
├─ src/           # implementación C++ (.cpp, .hpp)
├─ include/       # headers extra (si aplica)
├─ scripts/       # bash scripts (descarga, corridas, benchmarks)
├─ build/         # binarios compilados
├─ data/          
│  └─ dataset_grafos_no_dirigidos/
│     ├─ new_1000_dataset/
│     ├─ new_2000_dataset/
│     └─ new_3000_dataset/
├─ Makefile
└─ README.md
```

---

## 8) Resultados (qué esperar)

- **Greedy determinista:** soluciones estables y tiempos muy bajos (sub-ms para n=1000).  
- **Greedy aleatorizado (α=0.1):** resultados similares al determinista con variación por aleatoriedad.  
- A mayor densidad p, el tamaño del MIS disminuye (más aristas ⇒ más conflicto).  
- Los tiempos crecen con n, pero se mantienen en el rango de milisegundos.

---

## 9) Problemas frecuentes (FAQ)

- **`make: g++: command not found`** → Instalar dependencias: `sudo apt -y install build-essential`.  
- **Permisos en scripts** → `chmod +x scripts/*.sh`.  
- **Ruta dataset incorrecta** → Verifica que los `.graph` estén bajo `data/dataset_grafos_no_dirigidos/...`.  
- **Trabajar fuera de /home** → Evita clonar/compilar desde rutas de Windows (`\\wsl.localhost\...`).  

---

## 10) Licencia
MIT (opcional). Ver `LICENSE` si se incluye.
