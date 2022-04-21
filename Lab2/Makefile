CXXFLAGS+=-O2 -ggdb -DDEBUG
CXXFLAGS+=-Wall -Wextra

all: http-server

test: src/test.cc
	g++ -pthread -g -o $@ $^

http-server: src/*
	g++ -pthread -O2 -o $@ $^

debug:  src/*
	g++ -pthread -O0 -g -o $@ $^

clean: http-server
	rm -f ./http-server