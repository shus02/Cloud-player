CXXFLAGS+=-O2 -ggdb -DDEBUG
CXXFLAGS+=-Wall -Wextra

all: sudoku_solve

sudoku_solve: src/main.cc src/sudoku_dancing_links.cc src/neighbor.cc src/sudoku_basic.cc
	g++ -pthread -O2 -o $@ $^