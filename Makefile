.PHONY: all test clean debug hex_grid
	all: grid

CFLAGS= -g3 -std=c99 -O0 -rdynamic -Wall -fPIC -shared -Wno-missing-braces

hex_grid: hex_grid.so
hex_grid.so: luabinding.c hex_grid.c node_freelist.c intlist.c
	    gcc $(CFLAGS) -o $@ $^

all: test
test:
	lua test.lua

clean:
	rm -f *.so

debug: CFLAGS += -DDEBUG
debug: hex_grid test

