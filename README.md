# MIS 2025 — Heurísticas Greedy para Maximum Independent Set (MIS)

**Autores:** Matias Gayoso, Contanza Obreque  
**Entorno:** Windows + WSL2 (Ubuntu) + VSCode  
**Lenguaje:** C++17

---

## 1. Problema
El **Maximum Independent Set (MIS)** en grafos no dirigidos busca encontrar el conjunto independiente de mayor tamaño, es decir, el subconjunto de vértices tal que no exista arista entre ellos.

Este problema es **NP-difícil**, por lo que en este trabajo abordamos el desarrollo de **heurísticas Greedy** para obtener soluciones aproximadas en tiempos eficientes.

---

## 2. Heurísticas (Entrega 1)
- **Greedy determinista**  
  Selecciona iterativamente el vértice de **menor grado actual**, lo añade a la solución y elimina a sus vecinos del grafo residual.

- **Greedy aleatorizado (RCL-α)**  
  Construye una lista restringida de candidatos (RCL) con todos los vértices cuyo grado es menor o igual al umbral  
  d_min + α (d_max - d_min)  
  Luego selecciona uno de estos candidatos al azar. El parámetro **α** regula el nivel de aleatoriedad.

---

## 3. Requisitos de ejecución (según pauta)
La salida debe ser **en una sola línea**:  
```
<valor_objetivo> <tiempo_segundos>
```

### Comandos:
```bash
Greedy -i <instancia>
Greedy-probabilista -i <instancia> --alpha <0..1> --seed <int>
```

---

## 4. Compilación (WSL / Ubuntu)
Compilar con:
```bash
make
```

Se generan los binarios en `build/`:
- `build/Greedy`
- `build/Greedy-probabilista`

---

## 5. Ejecución (ejemplo)
```bash
./build/Greedy -i data/grafos/dataset_grafos_no_dirigidos/new_1000_dataset/erdos_n1000_p0c0.05_1.graph

./build/Greedy-probabilista -i data/grafos/dataset_grafos_no_dirigidos/new_1000_dataset/erdos_n1000_p0c0.05_1.graph --alpha 0.1 --seed 1
```

Salida esperada:
```
<valor> <tiempo>
```

---

## 6. Scripts de evaluación
- `scripts/run_one.sh` → Corre una instancia individual y muestra `<valor> <tiempo>`.
- `scripts/run_benchmark.sh` → Corre N veces una misma instancia y calcula promedios.
- `scripts/eval_folder.sh` → Recorre carpetas por (n,p), promedia sobre 30 instancias y exporta resultados en CSV.

---

## 7. Datos
El dataset **no se versiona** en este repositorio (archivos muy grandes).  
Colocar las instancias en `data/` y ajustar las rutas de ejemplo.

---

## 8. Estructura recomendada
```
mis-2025/
├─ src/           # Implementación C++ (.cpp, .hpp)
├─ include/       # Headers adicionales
├─ scripts/       # Bash scripts de corridas/benchmarks
├─ build/         # Binarios compilados (ignorado en Git)
├─ data/          # Datasets (ignorado en Git)
├─ Makefile
└─ README.md
```

---

## 9. Resultados esperados
- **Greedy determinista:** soluciones estables y tiempos muy bajos (sub-milisegundos).  
- **Greedy aleatorizado (α=0.1):** soluciones muy similares al determinista, con variación controlada por aleatoriedad.  
- A mayor densidad p, el tamaño del MIS disminuye.  
- Los tiempos crecen con n, pero se mantienen en el rango de milisegundos.

---

## 10. Licencia
MIT (opcional). Ver archivo `LICENSE` si se incluye.
