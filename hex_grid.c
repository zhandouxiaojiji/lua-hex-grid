#include "hex_grid.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef DEBUG
#define DBGprint(fmt, args...)                                              \
    fprintf(stderr, "\033[1;32m  ---- grid in c ----(%s:%d):\t\033[0m" fmt, \
            __func__, __LINE__, ##args)
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

#define DIR_W 0
#define DIR_NW 1
#define DIR_NE 2
#define DIR_E 3
#define DIR_SE 4
#define DIR_SW 5

#define idx2col(idx, w) ((idx) % (w))
#define idx2row(idx, w) ((idx) / (w))

static IntList* path = NULL;

NodeFreeList* hg_get_open_list(HexGrid* grid) {
    return grid->open_list;
}

// 32 位整数取较小值
/*
static int min_int(int a, int b) {
    a -= b;
    a &= a >> 31;
    return a + b;
}
*/

// 32 位整数取较大值
static int max_int(int a, int b) {
    a -= b;
    a &= (~a) >> 31;
    return a + b;
}

static int calc_hscore(HexBlock* from, HexBlock* to) {
    return max_int(max_int(abs(from->x - to->x), abs(from->y - to->y)),
                   abs(from->z - to->z));
}

static inline int in_dirs(unsigned char dirs, unsigned char dir) {
    return (dirs | (1 << dir)) == dirs;
}

static void add_to_path(HexBlock* block) {
    il_set(path, il_push_back(path), 0, block->idx);
}

static void set_dirty(HexGrid* grid, HexBlock* block) {
    il_set(grid->dirty_list, il_push_back(grid->dirty_list), 0, block->idx);
}

static inline int is_in_grid(HexGrid* grid, int col, int row) {
    return col >= 0 && col < grid->w && row >= 0 && row < grid->h;
}

static inline int is_close(HexBlock* block) {
    return block->flag == FLAG_CLOSE;
}

static inline int is_open(HexBlock* block) {
    return block->flag == FLAG_OPEN;
}

static void add_to_open_list(HexGrid* grid, HexBlock* src, HexBlock* dst) {
    if (dst->flag == FLAG_OPEN || dst->flag == FLAG_CLOSE) {
        return;
    }
    set_dirty(grid, dst);
    if (src != NULL) {
        dst->prev_idx = src->idx;
    }
    dst->flag = FLAG_OPEN;
    nfl_insert(grid->open_list, dst->idx, dst->gscore, dst->hscore);

#ifdef DEBUG
    int fscore = dst->gscore + dst->hscore;
    DBGprint("=========open (%d %d) g:%d h:%d f:%d prev:(%d, %d)==========\n",
             dst->col, dst->row, dst->gscore, dst->hscore, fscore, src ? src->col : -1,
             src ? src->row : -1);
    // hg_dump(grid);
#endif
}

/*
static HexBlock* get_block_by_offset(HexGrid* grid, int col, int row) {
    col = min_int(grid->w - 1, max_int(0, col));
    row = min_int(grid->h - 1, max_int(0, row));
    return grid->blocks[col + row * grid->w];
}
*/

static int oddr_directions[2][6][2] = {
    {{-1, 0}, {-1, -1}, {0, -1}, {1, 0}, {0, 1}, {-1, 1}},
    {{-1, 0}, {0, -1}, {1, -1}, {1, 0}, {1, 1}, {0, 1}},
};

static HexBlock* get_neighbor(HexGrid* grid, HexBlock* block, int dir) {
    int parity = block->row & 1;
    int* arr = oddr_directions[parity][dir];
    int col = block->col + arr[0];
    int row = block->row + arr[1];
    // printf("get_neighbor, block:(%d %d), dir:%d, (%d, %d)\n", block->col,
    // block->row, dir, col, row);
    if (is_in_grid(grid, col, row)) {
        return grid->blocks[col + row * grid->w];
    } else {
        return NULL;
    }
}

static inline int is_terrain_obs(HexBlock* block) {
    return block == NULL || block->obstacle == OBS_TERRAIN;
}

static inline int is_obstacle(HexBlock* block, HexBlock* start, HexBlock* end, int ignore_lv) {
    if((block == start || block == end) && ignore_lv < OBS_BUILDING) {
        ignore_lv = OBS_BUILDING; // 普通寻路忽略起点和终点的阻挡
    }
    return block->obstacle > ignore_lv;
}

void hg_init() {
    path = (IntList*)malloc(sizeof(IntList));
    il_init(path, 1);
}

void hg_create(HexGrid* grid, int w, int h) {
    int count = w * h;
    grid->blocks = malloc(count * sizeof(HexBlock*));

    for (int i = 0; i < count; ++i) {
        HexBlock* block = (HexBlock*)malloc(sizeof(HexBlock));
        block->obstacle = 0;
        block->idx = i;
        block->col = idx2col(i, w);
        block->row = idx2row(i, w);
        block->x = block->col - (block->row - (block->row & 1)) / 2;
        block->z = block->row;
        block->y = -block->x - block->z;
        block->prev_idx = -1;
        block->flag = FLAG_INIT;
        grid->blocks[i] = block;
    }

    grid->w = w;
    grid->h = h;
    grid->sb = NULL;
    grid->eb = NULL;

    grid->open_list = (NodeFreeList*)malloc(sizeof(NodeFreeList));
    nfl_init(grid->open_list);

    grid->dirty_list = (IntList*)malloc(sizeof(IntList));
    il_init(grid->dirty_list, 1);

    DBGprint("create grid(w:%d,h:%d)\n", w, h);
}

void hg_destroy(HexGrid* grid) {
    for (int i = 0; i < grid->w * grid->h; ++i) {
        free(grid->blocks[i]);
    }
    free(grid->blocks);

    nfl_destroy(grid->open_list);
    free(grid->open_list);

    il_destroy(grid->dirty_list);
    free(grid->dirty_list);
}

void hg_set_obstacle(HexGrid* grid, int pos, int obstacle) {
    HexBlock* block = grid->blocks[pos];
    block->obstacle = obstacle;
}

int hg_get_area(HexGrid* grid, int pos) {
    HexBlock* block = grid->blocks[pos];
    return block->area;
}

static void set_area(HexGrid* grid, HexBlock* block, int area) {
    block->area = area;
    for (int dir = 0; dir < NO_DIRECTION; ++dir) {
        HexBlock* neighbor = get_neighbor(grid, block, dir);
        if (neighbor != NULL && neighbor->area == 0 && !is_terrain_obs(neighbor)) {
            set_area(grid, neighbor, area);
        }
    }
}

void hg_update_area(HexGrid* grid) {
    for (int i = 0; i < grid->w * grid->h; ++i) {
        grid->blocks[i]->area = 0;
    }

    int area = 1;
    for (int i = 0; i < grid->w * grid->h; ++i) {
        HexBlock* block = grid->blocks[i];
        if (!is_terrain_obs(block) && block->area == 0) {
            set_area(grid, block, area++);
        }
    }
}

int hg_get_obstacle(HexGrid* grid, int pos) {
    if (pos < 0 || pos >= grid->w * grid->h) {
        return OBS_TERRAIN;
    }
    return grid->blocks[pos]->obstacle;
}

// static int FORCE_DIRS[][4] = {
//     {DIR_NW, DIR_NE, DIR_SW, DIR_SE},  // DIR_W  0
//     {DIR_NE, DIR_E, DIR_W, DIR_SW},    // DIR_NW 1
//     {DIR_NW, DIR_W, DIR_E, DIR_SE},    // DIR_NE 2
//     {DIR_NE, DIR_NW, DIR_SE, DIR_SW},  // DIR_E  3
//     {DIR_E, DIR_NE, DIR_SW, DIR_W},    // DIR_SE 4
//     {DIR_SE, DIR_E, DIR_W, DIR_NW},    // DIR_SW 5
// };

static void reset_grid(HexGrid* grid) {
    IntList* dirty_list = grid->dirty_list;
    for (int i = 0; i < dirty_list->num; ++i) {
        HexBlock* block = grid->blocks[il_get(dirty_list, i, 0)];
        block->prev_idx = -1;
        block->flag = FLAG_INIT;
    }
    il_clear(dirty_list);
    nfl_clear(grid->open_list);
    grid->sb = NULL;
    grid->eb = NULL;
}

// A-star
IntList* hg_pathfinding(HexGrid* grid, int pos1, int pos2, int ignore_lv) {
    // DBGprint("pathfinding (%d, %d) => (%d, %d)\n", x1, y1, x2, y2);
    HexBlock* start = grid->blocks[pos2]; // 交换起点终点
    HexBlock* end = grid->blocks[pos1];

    il_clear(path);
    if (start->area != end->area || is_terrain_obs(start) || is_terrain_obs(end)) {
        DBGprint("no way, start:%d, end:%d, area:%d\n", is_terrain_obs(start),
                 is_terrain_obs(end), start->area != end->area);
        return path;
    }
    if (start == end) {
        add_to_path(start);
        return path;
    }

    grid->sb = start;
    grid->eb = end;

    NodeFreeList* open_list = grid->open_list;
    add_to_open_list(grid, NULL, start);

    int gscore = 0;
    while (!nfl_is_empty(open_list)) {
        Node* node = nfl_pop(open_list);
        HexBlock* cur = grid->blocks[node->pos];
        cur->flag = FLAG_CLOSE;
        if (cur == end) {
            // end->prev_idx = cur->idx;
            break;
        }
        gscore++;
        // printf("current: (%d %d), g:%d, h:%d, f:%d\n", node->idx % grid->w,
        // node->idx / grid->w, node->g, node->h, node->f);
        for (int dir = 0; dir < NO_DIRECTION; ++dir) {
            HexBlock* block = get_neighbor(grid, cur, dir);
            if (block == NULL) {
                continue;
            }
            if (block->flag == FLAG_INIT) {
                if (is_obstacle(block, start, end, ignore_lv)) {
                    block->flag = FLAG_CLOSE;
                    set_dirty(grid, block);
                } else {
                    block->gscore = gscore;
                    block->hscore = calc_hscore(block, grid->eb);
                    add_to_open_list(grid, cur, block);
                }
            } else if (block->flag == FLAG_OPEN && block->gscore > gscore + 1) {
                nfl_remove(grid->open_list, block->node);
                block->gscore = gscore + 1;
                nfl_insert(grid->open_list, block->idx, block->gscore,
                           block->hscore);
            }
        }
    }

    // hg_dump(grid);

    if (end->prev_idx >= 0) {
        int idx = end->idx;
        while (idx >= 0) {
            HexBlock* cur = grid->blocks[idx];
            add_to_path(cur);
            if (idx == cur->prev_idx) {
                printf("endless idx:%d\n", idx);
                break;
            }
            idx = cur->prev_idx;
        }
    }

    reset_grid(grid);

    return path;
}

void hg_dump(HexGrid* grid) {
    int w = grid->w;
    int h = grid->h;
    for (int y = 0; y < h; ++y) {
        if (y % 2 != 0)
            printf(" ");
        for (int x = 0; x < w; ++x) {
            HexBlock* block = grid->blocks[y * grid->w + x];
            if (block->obstacle == OBS_BUILDING)
                printf("@ ");
            else if (block->obstacle == OBS_TERRAIN)
                printf("X ");
            else
                printf("%d ", block->obstacle);
            /*
            HexBlock* cur = grid->blocks[y * grid->w + x];
            if (cur->prev_idx < 0){
                printf("(%d,%d)           ", x, y);
            } else {
                HexBlock* prev = grid->blocks[cur->prev_idx];
                printf("(%d,%d)-(%d,%d)   ", x, y, prev->col, prev->row);
            }
            */
        }
        printf("\n");
    }
}
