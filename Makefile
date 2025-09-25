# Makefile para compilar los binarios Greedy y Greedy-probabilista
# Uso:
#   make        → compila los ejecutables en build/
#   make clean  → elimina binarios

CXX = g++
CXXFLAGS = -std=c++17 -O2 -Wall
SRC = src
BUILD = build

all: $(BUILD)/Greedy $(BUILD)/Greedy-probabilista

$(BUILD)/Greedy: $(SRC)/greedy.cpp $(SRC)/graph_io.hpp $(SRC)/utils.hpp
	mkdir -p $(BUILD)
	$(CXX) $(CXXFLAGS) -o $@ $(SRC)/greedy.cpp

$(BUILD)/Greedy-probabilista: $(SRC)/greedy_rand.cpp $(SRC)/graph_io.hpp $(SRC)/utils.hpp
	mkdir -p $(BUILD)
	$(CXX) $(CXXFLAGS) -o $@ $(SRC)/greedy_rand.cpp

clean:
	rm -rf $(BUILD)
