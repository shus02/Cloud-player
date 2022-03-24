CXXFLAGS+=-O2 -ggdb -DDEBUG
CXXFLAGS+=-Wall -Wextra

all: sudoku_solve

sudoku_solve: src/main.cc src/sudoku_dancing_links.cc src/neighbor.cc src/sudoku_basic.cc src/io.cc src/sudoku.h
	g++ -pthread -O2 -o $@ $^

debug: src/main.cc src/sudoku_dancing_links.cc src/neighbor.cc src/sudoku_basic.cc src/io.cc src/sudoku.h
	g++ -pthread -g -o $@ $^

test: src/test.cc
	g++ -pthread -g -o $@ $^