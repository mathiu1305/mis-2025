CXX = g++
CXXFLAGS = -O3 -std=c++17 -Wall -Wextra
BIN = build
SRC = src

all: $(BIN)/Greedy $(BIN)/Greedy-probabilista

$(BIN)/Greedy: $(SRC)/greedy.cpp $(SRC)/utils.hpp
	mkdir -p $(BIN)
	$(CXX) $(CXXFLAGS) $(SRC)/greedy.cpp -o $@

$(BIN)/Greedy-probabilista: $(SRC)/greedy_rand.cpp $(SRC)/utils.hpp
	mkdir -p $(BIN)
	$(CXX) $(CXXFLAGS) $(SRC)/greedy_rand.cpp -o $@

clean:
	rm -rf $(BIN)
