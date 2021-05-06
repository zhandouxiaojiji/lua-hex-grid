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

// N, NE, E, SE, S, SW, W, NW
/*
    1    2
     \  /
  0 --  -- 3
     /  \
    5    4
*/
#define NO_DIRECTION 6
#define FULL_DIRECTIONSET 63
#define EMPTY_DIRECTIONSET 0

#define DIR_W  0
#define DIR_NW 1
#define DIR_NE 2
#define DIR_E  3
#define DIR_SE 4
#define DIR_SW 5

#define idx2col(idx, w) idx % w
#define idx2row(idx, w) idx / w
#define is_close(block) block->dirs == FULL_DIRECTIONSET

NodeFreeList* hg_get_open_list(HexGrid* grid) {
    return grid->open_list;
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

static void dir_add(unsigned char *dirs, unsigned char dir){
    *dirs |= (1 << dir);
}

static inline int in_dirs(unsigned char dirs, unsigned char dir) {
    return (dirs | (1<<dir)) == dirs;
}

static void add_to_path(HexGrid* grid, HexBlock* block) {
    int i = il_push_back(grid->path);
    il_set(grid->path, i, 0, block->idx);
}

static void set_dir(HexGrid* grid, HexBlock* block, int dir) {
    int i = il_push_back(grid->dirty_list);
    il_set(grid->dirty_list, i, 0, block->idx);
    block->dirs |= (1<<dir);
}

static inline int is_in_grid(HexGrid* grid, int col, int row) {
    return col < 0 || col >= grid->w || row < 0 || row >= grid->h;
}

static HexBlock* get_block_by_offset(HexGrid* grid, int col, int row) {
    col = min_int(grid->w - 1, max_int(0, col));
    row = min_int(grid->h - 1, max_int(0, row));
    return grid->blocks[col + row * grid->w];
}

static inline int walkable(HexBlock* block, int camp) {
    return block->obstacle == 0 || block->obstacle == camp;
}

static int calc_h(HexBlock* from, HexBlock* to) {
    return max_int(max_int(abs(from->x - to->x), abs(from->y - to->y)), abs(from->z - to->z));
}

void hg_init() {

}

void hg_create(HexGrid* grid, int w, int h) {
    int count = w * h;
    grid->blocks = malloc(count * sizeof(HexBlock*));

    for(int i = 0; i < count; ++i) {
        HexBlock* block = (HexBlock*)malloc(sizeof(HexBlock));
        block->obstacle = 0;
        block->idx = i;
        block->col = idx2col(i, w);
        block->row = idx2row(i, w);
        block->x = block->col - (block->row + ( block->row & 1)) / 2;
        block->z = block->row;
        block->y = -block->x - block->z;
        grid->blocks[i] = block;
    }

    grid->w = w;
    grid->h = h;

    grid->open_list = nfl_create();
    grid->dirty_list = il_create(1);
    grid->path = il_create(1);

    DBGprint("create grid(w:%d,h:%d)\n", w, h);
}

void hg_destroy(HexGrid* grid) {
    free(grid->blocks);
}

void hg_set(HexGrid* grid, int x, int y, int obstacle) {
    DBGprint("set (%d, %d) = %d\n", x, y, obstacle);
    HexBlock* block = get_block_by_offset(grid, x, y);
    block->obstacle = obstacle;
}

static HexBlock* get_neighbor(HexGrid* grid, HexBlock* block, int dir) {
    int col = block->col;
    int row = block->row;
    if(dir == DIR_E){
        col++;
    } else if (dir == DIR_W) {
        col--;
    } else if (dir == DIR_NE) {
        col++;
        row--;
    } else if (dir == DIR_NW) {
        row--;
    } else if (dir == DIR_SE) {
        col++;
        row++;
    } else if(dir == DIR_SW) {
        row++;
    }
    if(is_in_grid(grid, col, row)) {
        return grid->blocks[col + row * grid->w];
    } else {
        return NULL;
    }
}

static int find_forced_neighbor(HexGrid* grid, HexBlock* block, int from_dir) {

}

static int find_jump_point(HexGrid* grid, HexBlock* src, int dir, int g) {
    HexBlock* next = get_neighbor(grid, src, dir);
    if(!next) {
        return 0;
    }

    return 1;
}

static int search_block(HexGrid* grid, HexBlock* block, int g) {
    for(int dir = 0; dir < NO_DIRECTION; ++dir) {
        if(!in_dirs(block->dirs, dir)) {
            set_dir(grid, block, dir);
            if(find_jump_point(grid, block, dir, g)){
                return 1;
            }
        }
    }
    return 0;
}

static void reset_dirs(HexGrid* grid) {
    IntList* dirty_list = grid->dirty_list;
    for(int i = 0; i < dirty_list->num; ++i) {
        HexBlock* block = grid->blocks[il_get(dirty_list, i, 0)];
        block->dirs = 0;
    }
    il_clear(dirty_list);
}

IntList* hg_pathfinding(HexGrid* grid, int c1, int r1, int c2, int r2, int camp) {
    //DBGprint("pathfinding (%d, %d) => (%d, %d)\n", x1, y1, x2, y2);
    HexBlock* start = get_block_by_offset(grid, c1, r1);
    HexBlock* end = get_block_by_offset(grid, c2, r2);
    grid->sb = start;
    grid->eb = end;

    IntList* path = grid->path;
    il_clear(path);
    if(!walkable(start, camp) || !walkable(end, camp)) {
        return path;
    }
    add_to_path(grid, start);
    if(start == end) {
        return path;
    }

    NodeFreeList* open_list = grid->open_list;
    nfl_clear(open_list);
    nfl_insert(open_list, start->idx, 0, calc_h(start, end));

    int limit = 10;
    while(!nfl_is_empty(open_list) && limit-- > 0) {
        Node* node = nfl_head(open_list);
        HexBlock* block = grid->blocks[node->idx];
        printf("current:%d, g:%d, h:%d\n", node->idx, node->g, node->h);
        search_block(grid, block, node->g);
        if(is_close(block)) {
            nfl_pop(open_list);
        }
    }
    reset_dirs(grid);
    return path;
}

void hg_dump(HexGrid* grid) {
    int w = grid->w;
    int h = grid->h;
    for(int y = 0; y < h; ++ y){
        if(y%2!=0)
            printf(" ");
        for(int x = 0; x < w; ++x){
            int obstacle = grid->blocks[y * grid->w + x]->obstacle;
            if(obstacle > 0)
                printf("x ");
            else
                printf("* ");
        }
        printf("\n");
    }
}
