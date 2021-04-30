#include "hex_grid.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

void dump_grid(HexGrid* grid) {

}

#ifdef DEBUG
#include <stdio.h>
#define DBGprint(fmt, args...) fprintf(stderr, "\033[1;32m  ---- grid in c ----(%s:%d):\t\033[0m" fmt, __func__, __LINE__, ##args)
#define DBDump(grid) dump_grid(grid)
#else
#define DBGprint(fmt, args...)
#define DBDump(grid)
#endif

// 32 位整数取较小值
static int min_int(int a, int b) {
    a -= b;
    a &= a >> 31;
    return a + b;
}

// 32 位整数取较大值
static int max_int(int a, int b) {
    a -= b;
    a &= (~a) >> 31;
    return a + b;
}

// 计算一行或者一列内的网格索引
static int to_cell_idx(int x, int y, int w, int h) {
    x = min_int(w - 1, max_int(0, x));
    y = min_int(h - 1, max_int(0, y));
    return x + y * w;
}

void hg_create(HexGrid* grid, int w, int h) {
    grid->cells = (int *)malloc(w * h * sizeof(int));
    grid->w = w;
    grid->h = h;
    DBGprint("create grid(w:%d,h:%d)\n", w, h);
    DBDump(grid);
}

void hg_destroy(HexGrid* grid) {
    free(grid->cells);
}

void hg_set(HexGrid* grid, int x, int y, int obstacles) {
    DBGprint("set (%d, %d) = %d\n", x, y, obstacles);
    int idx = to_cell_idx(x, y, grid->w, grid->h);
    grid->cells[idx] = obstacles;
}

void hg_pathfinding(HexGrid* grid, int x1, int y1, int x2, int y2) {
    DBGprint("pathfinding (%d, %d) => (%d, %d)\n", x1, y1, x2, y2);
}

