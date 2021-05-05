#include "hex_grid.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifdef DEBUG
    #define DBGprint(fmt, args...) fprintf(stderr, "\033[1;32m  ---- grid in c ----(%s:%d):\t\033[0m" fmt, __func__, __LINE__, ##args)
#else
    #define DBGprint(fmt, args...)
#endif

static IntList* close_list = NULL;
static NodeFreeList* open_list = NULL;
static IntList* path = NULL;

NodeFreeList* hg_get_open_list() {
    return open_list;
}

IntList* hg_get_close_list() {
    return close_list;
}

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

static void add_to_path(int pos) {
    int i = il_push_back(path);
    il_set(path, i, 0, pos);
}

// 计算一行或者一列内的网格索引
static int to_block_pos(int x, int y, int w, int h) {
    x = min_int(w - 1, max_int(0, x));
    y = min_int(h - 1, max_int(0, y));
    return x + y * w;
}

void hg_init() {
    open_list = nfl_create();
    close_list = il_create(1);
    path = il_create(1);
}

void hg_create(HexGrid* grid, int w, int h) {
    grid->blocks = (int *)malloc(w * h * sizeof(int));
    grid->w = w;
    grid->h = h;

    DBGprint("create grid(w:%d,h:%d)\n", w, h);
}

void hg_destroy(HexGrid* grid) {
    free(grid->blocks);
}

void hg_set(HexGrid* grid, int x, int y, int obstacles) {
    DBGprint("set (%d, %d) = %d\n", x, y, obstacles);
    int idx = to_block_pos(x, y, grid->w, grid->h);
    grid->blocks[idx] = obstacles;
}

IntList* hg_pathfinding(HexGrid* grid, int x1, int y1, int x2, int y2) {
    DBGprint("pathfinding (%d, %d) => (%d, %d)\n", x1, y1, x2, y2);
    int start = to_block_pos(x1, y1, grid->w, grid->h);
    int end = to_block_pos(x2, y2, grid->w, grid->h);
    il_clear(path);
    add_to_path(start);
    add_to_path(end);
    if(start == end) {
        return path;
    }
    return path;
}

void hg_dump(HexGrid* grid) {
    int w = grid->w;
    int h = grid->h;
    for(int y = 0; y < h; ++ y){
        if(y%2!=0)
            printf(" ");
        for(int x = 0; x < w; ++x){
            int obstacles = grid->blocks[to_block_pos(x, y, w, h)];
            if(obstacles > 0)
                printf("x ");
            else
                printf("* ");
        }
        printf("\n");
    }
}
