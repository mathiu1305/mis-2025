# ======================================================
#  Makefile — Proyecto MIS 2025
#  Compila los binarios: Greedy, Greedy-probabilista y SA
# ======================================================

CXX = g++
CXXFLAGS = -std=c++17 -O2 -Wall
CXXFLAGS_RELEASE = -std=c++17 -O3 -Wall -DNDEBUG
SRC = src
BUILD = build

# ======================================================
#  Reglas principales
# ======================================================

all: $(BUILD)/Greedy $(BUILD)/Greedy-probabilista $(BUILD)/SA
	@echo "\033[1;32m✅ Compilación completada correctamente.\033[0m"

# --- Greedy determinista ---
$(BUILD)/Greedy: $(SRC)/greedy.cpp $(SRC)/graph_io.hpp $(SRC)/utils.hpp
	@mkdir -p $(BUILD)
	@echo "\033[1;36m→ Compilando Greedy determinista...\033[0m"
	@$(CXX) $(CXXFLAGS) -o $@ $(SRC)/greedy.cpp
	@echo "\033[1;32m✔ Greedy compilado correctamente.\033[0m\n"

# --- Greedy probabilista ---
$(BUILD)/Greedy-probabilista: $(SRC)/greedy_rand.cpp $(SRC)/graph_io.hpp $(SRC)/utils.hpp
	@mkdir -p $(BUILD)
	@echo "\033[1;36m→ Compilando Greedy aleatorizado...\033[0m"
	@$(CXX) $(CXXFLAGS) -o $@ $(SRC)/greedy_rand.cpp
	@echo "\033[1;32m✔ Greedy-probabilista compilado correctamente.\033[0m\n"

# --- Metaheurística SA ---
$(BUILD)/SA: $(SRC)/SA.cpp
	@mkdir -p $(BUILD)
	@echo "\033[1;36m→ Compilando Simulated Annealing (SA)...\033[0m"
	@$(CXX) $(CXXFLAGS_RELEASE) -o $@ $(SRC)/SA.cpp
	@echo "\033[1;32m✔ SA compilado correctamente.\033[0m\n"

# ======================================================
#  Limpieza
# ======================================================

clean:
	@echo "\033[1;33m🧹 Limpiando archivos compilados...\033[0m"
	@rm -rf $(BUILD)
	@echo "\033[1;32m✔ Limpieza completada.\033[0m\n"

# ======================================================
#  Ayuda
# ======================================================

help:
	@echo "\033[1;34mUso:\033[0m"
	@echo "  make              → Compila todos los ejecutables"
	@echo "  make clean        → Elimina los binarios generados"
	@echo "  make help         → Muestra esta ayuda"
