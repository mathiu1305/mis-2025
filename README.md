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

trabaja siempre dentro de `/home/<usuario>/...` (Linux).  
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


======================================================================
# ENTREGA 2 — Metaheurística de Trayectoria: Simulated Annealing (SA)
======================================================================

**Autores:** Matías Gayoso, Constanza Obreque  
**Entorno:** Ubuntu (WSL2) + VSCode  
**Lenguaje:** C++17

---

### Descripción general
En esta entrega se implementa la metaheurística **Simulated Annealing (SA)** para el problema MIS.  
El algoritmo utiliza un vecindario de inserción con completación local y criterio de aceptación probabilístico,  
mostrando mejoras *any-time* durante la ejecución y una línea final con el mejor resultado y tiempo.

---

## 8) Compilación

### Opción A — con Makefile
```bash
cd ~/mis-2025
make SA
```
Si quieres recompilar desde cero:
```bash
make clean && make SA
```

### Opción B — compilación manual
```bash
cd src
g++ -O2 -std=c++17 SA.cpp -o SA
```
Ejecutable resultante: `./SA`

---

## 9) Ejecución básica

Formato de salida (*any-time*):
```
<mejor_tamaño> <tiempo_segundos>
```
Además se muestran líneas de configuración y estadísticas:
```
#config: seed=... T0=... alpha=... diversify=off adaptive_temp=off verbose=off
#stats: total_moves=... accepted=... rate=... improvements=... moves_per_sec=...
```

### Ejemplos rápidos (≤ 3 s)

> **N = 1000**
```bash
./build/SA -i data/dataset_grafos_no_dirigidos/new_1000_dataset/erdos_n1000_p0c0.1_3.graph -t 3
./build/SA -i data/dataset_grafos_no_dirigidos/new_1000_dataset/erdos_n1000_p0c0.5_5.graph -t 3
./build/SA -i data/dataset_grafos_no_dirigidos/new_1000_dataset/erdos_n1000_p0c0.9_2.graph -t 3
```

> **N = 2000**
```bash
./build/SA -i data/dataset_grafos_no_dirigidos/new_2000_dataset/erdos_n2000_p0c0.1_6.graph -t 3
./build/SA -i data/dataset_grafos_no_dirigidos/new_2000_dataset/erdos_n2000_p0c0.5_8.graph -t 3
./build/SA -i data/dataset_grafos_no_dirigidos/new_2000_dataset/erdos_n2000_p0c0.9_11.graph -t 3
```

> **N = 3000**
```bash
./build/SA -i data/dataset_grafos_no_dirigidos/new_3000_dataset/erdos_n3000_p0c0.1_3.graph -t 3
./build/SA -i data/dataset_grafos_no_dirigidos/new_3000_dataset/erdos_n3000_p0c0.5_10.graph -t 3
./build/SA -i data/dataset_grafos_no_dirigidos/new_3000_dataset/erdos_n3000_p0c0.9_18.graph -t 3
```

> **Nota:** si el ejecutable está en `src/`, usa `./src/SA` en vez de `./build/SA`.

---

## 10) Parámetros principales

| Parámetro | Por defecto | Descripción |
|------------|:-----------:|-------------|
| `-i` | (required) | Ruta del grafo `.graph` |
| `-t` | (required) | Tiempo límite en segundos |
| `--seed S` | reloj SO | Semilla para reproducibilidad |
| `--T0 v` | 1.0 | Temperatura inicial |
| `--alpha v` | 0.999 | Factor de enfriamiento |
| `--iters_per_T v` | 1000 | Iteraciones por temperatura |

**Recomendado:** `--T0 2.0 --alpha 0.9995` para instancias grandes.  
Usa `--seed` para corridas repetibles.

---

## 11) Interpretación de la salida

Ejemplo real:
```
#config: seed=1760740917462159746 T0=2 alpha=0.9995 diversify=off adaptive_temp=off verbose=off
#stats: total_moves=104129560 accepted=3278 rate=0.000 improvements=1462 moves_per_sec=34709689
1526 0.126138
```

- `#config` → parámetros efectivos de la corrida.  
- `#stats` → movimientos totales, aceptados, mejoras y velocidad en mov/s.  
- Última línea → mejor tamaño de conjunto y tiempo de obtención.

---

## 12) Errores comunes

**Ruta inválida**
```
ERROR: Cannot open file: <ruta>
```
**Bandera no soportada**
```
Unknown or incomplete arg: --adaptive
```

---

## 13) Conclusiones

El Simulated Annealing mejora de 25–60 % las soluciones Greedy según la densidad del grafo.  
La ejecución es estable, eficiente y muestra progreso any-time en tiempo real,  
con tiempos subcuadráticos incluso para N = 3000.


============================================================
# ENTREGA 3 — Metaheurística Poblacional: Algoritmo Genético (GA)
============================================================

## 14) Compilación

```bash
make GA
```

o manualmente:

```bash
g++ -O3 -std=c++17 src/ga/GA.cpp -o build/GA
```

---

## 15) Ejecución del GA (criterio Any-Time)

El GA imprime una línea **cada vez que encuentra una mejor solución**:

```
<mejor_tamaño> <tiempo>
```

Ejemplo:

```bash
./build/GA \
  -i data/.../erdos_n3000_p0c0.5_1.graph \
  -t 10 \
  --pop 100 --pc 0.8 --pm 0.05 \
  --elitism 1 --init mix --ls 1
```

---

## 16) Parámetros principales del GA

| Parámetro | Descripción |
|----------|-------------|
| `-i` | Instancia `.graph` |
| `-t` | Tiempo límite |
| `--pop` | Tamaño de población |
| `--pc` | Probabilidad de cruce |
| `--pm` | Probabilidad de mutación |
| `--elitism` | Nº de individuos élite |
| `--stall_gen` | Máximo de generaciones sin mejora |
| `--init` | Inicialización (`random`, `greedy`, `mix`) |
| `--ls` | Búsqueda local (0/1) |

---

## 17) Tuning automático con IRACE (en clúster Luthier)

Los archivos se encuentran en:

```
tuning/ga/
├─ scenario.txt
├─ instances-list.txt
├─ parameter_definitions.txt
└─ runner.sh
```

Ejecución:

```bash
cd tuning/ga
irace --scenario scenario.txt
```

### 📌 Mejor configuración encontrada por IRACE

```
--pop 129
--pc 0.9955
--pm 0.1512
--elitism 1
--stall_gen 101
--init mix
--ls 1
```

---

## 18) Scripts oficiales para la evaluación del GA (10s y 60s)

Los scripts nuevos de esta entrega se ubican en:

```
scripts/ga/run_ga_10s.sh
scripts/ga/run_ga_60s.sh
scripts/ga/run_ga_miniset.sh
```

Permiten:

- ejecutar GA sobre una instancia  
- ejecutar GA 10s/60s  
- comparar una minimuesta (N=1000/2000/3000 con p=0.1, 0.5, 0.9)

Ejemplo:

```bash
scripts/ga/run_ga_miniset.sh build/GA
```

---

## 19) Mini-set usado para comparaciones 10s vs 60s

```
# N=1000
erdos_n1000_p0c0.1_1.graph
erdos_n1000_p0c0.5_1.graph
erdos_n1000_p0c0.9_1.graph

# N=2000
erdos_n2000_p0c0.1_1.graph
erdos_n2000_p0c0.5_1.graph
erdos_n2000_p0c0.9_1.graph

# N=3000
erdos_n3000_p0c0.1_1.graph
erdos_n3000_p0c0.5_1.graph
erdos_n3000_p0c0.9_1.graph
```

---

## 20) Conclusión de Entrega 3

- El GA presenta **estabilidad alta** y **baja varianza** comparado con SA.  
- Con la configuración óptima de IRACE, **GA-60s supera consistentemente a GA-10s**.  
- La política any-time facilita la comparación directa con SA.  
- Futuro trabajo: hibridación GA+SA.

---

============================================================
# ENTREGA 4 — Hibridación MH_p + MH_t: GA + Búsqueda Local (GA+LS)
============================================================

Autores: Matías Gayoso, Constanza Obreque  
Entorno: Ubuntu (WSL2 / Clúster Luthier)  
Lenguaje: C++17  

------------------------------------------------------------
## 21) Descripción general
------------------------------------------------------------

En esta entrega se implementa una **metaheurística híbrida** que combina:

- **MH poblacional (MH_p):** Algoritmo Genético (GA)
- **MH de trayectoria (MH_t):** Búsqueda Local (Local Search, LS)

La estrategia consiste en ejecutar un GA clásico y aplicar **búsqueda local periódica**
sobre individuos de la población, logrando un mejor equilibrio entre:
- exploración global,
- intensificación local,
- calidad de soluciones y comportamiento any-time.

El algoritmo final se denomina **GA+LS**.

------------------------------------------------------------
## 22) Compilación del solver final
------------------------------------------------------------

Desde la raíz del proyecto:

```bash
make solver_MISP
```

Compilación manual (alternativa):

```bash
g++ -O3 -std=c++17 src/final/solver_MISP.cpp -o build/solver_MISP
```

------------------------------------------------------------
## 23) Ejecución básica (Any-Time)
------------------------------------------------------------

```bash
./build/solver_MISP -i <instancia.graph> -t <segundos>
```

Ejemplo:

```bash
./build/solver_MISP \
  -i data/dataset_grafos_no_dirigidos/new_1000_dataset/erdos_n1000_p0c0.05_1.graph \
  -t 5
```

Salida:

```
<mejor_tamaño> <tiempo_desde_inicio>
```

------------------------------------------------------------
## 24) Parámetros del GA+LS
------------------------------------------------------------

| Parámetro | Descripción |
|---------|------------|
| `--pop` | Tamaño de población |
| `--ls_budget` | Presupuesto de búsqueda local |
| `--ls_freq` | Frecuencia de aplicación de LS |
| `-t` | Tiempo límite (segundos) |

------------------------------------------------------------
## 25) Tuning automático con IRACE
------------------------------------------------------------

Archivos utilizados:

```
tuning/final/
├─ scenario.txt
├─ parameter_definitions.txt
├─ runner.sh
└─ logs/irace.log
```

Ejecución:

```bash
irace --scenario tuning/final/scenario.txt
```

### Mejor configuración encontrada

```
--pop 79
--ls_budget 444
--ls_freq 5
```

------------------------------------------------------------
## 26) Evaluación experimental
------------------------------------------------------------

- Dataset total: **1710 instancias**
- Tamaños: N = 1000, 2000, 3000
- Probabilidades: p ∈ {0.1, 0.2, …, 0.9}

Ejecución masiva:

```bash
nohup scripts/run_ga_ls_final.sh > results/final_ga_ls/run.log 2>&1 &
```

Resultados:

```
results/final_ga_ls/ga_ls_final.csv
```

------------------------------------------------------------
## 27) Agregación de resultados
------------------------------------------------------------

```bash
python3 scripts/aggregate_ga_ls.py
```

Archivo generado:

```
ga_ls_aggregated.csv
```

------------------------------------------------------------
## 28) Resultados y comparación
------------------------------------------------------------

El algoritmo **GA+LS** supera consistentemente a:

- Greedy
- A-Greedy
- Simulated Annealing
- GA puro

en calidad de solución, manteniendo tiempos competitivos.

Resultados completos:
- `ga_ls_aggregated.csv`
- `final_comparative_results.csv`

------------------------------------------------------------
## 29) Conclusiones
------------------------------------------------------------

- La hibridación MH_p + MH_t entrega mejoras sustanciales.
- IRACE permite una calibración robusta de parámetros.
- GA+LS es la mejor solución final del proyecto.
