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

#define idx2col(idx, w) ((idx) % (w))
#define idx2row(idx, w) ((idx) / (w))

static IntList* path = NULL;

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

static int calc_h(HexBlock* from, HexBlock* to) {
    return max_int(max_int(abs(from->x - to->x), abs(from->y - to->y)), abs(from->z - to->z));
}

static inline int in_dirs(unsigned char dirs, unsigned char dir) {
    return (dirs | (1<<dir)) == dirs;
}

static void add_to_path(HexBlock* block) {
    il_set(path, il_push_back(path), 0, block->idx);
}

static void set_dirty(HexGrid* grid, HexBlock* block) {
    il_set(grid->dirty_list, il_push_back(grid->dirty_list), 0, block->idx);
}

static void add_dir(HexGrid* grid, HexBlock* block, int dir) {
    if(block->dirs == 0) {
        set_dirty(grid, block);
    }
    block->dirs |= (1<<dir);
}

static inline int is_in_grid(HexGrid* grid, int col, int row) {
    return col >= 0 && col < grid->w && row >= 0 && row < grid->h;
}

static inline int is_close(HexBlock* block) {
    return block->dirs == FULL_DIRECTIONSET;
}

static inline int is_open(HexBlock* block) {
    return block->isopen == 1;
}

static void add_to_open_list(HexGrid* grid, HexBlock* src, HexBlock* dst, int g) {
    set_dirty(grid, dst);
    if(src != NULL) {
        dst->prev_idx = src->idx;
    }
    dst->isopen = 1;
    if(!is_close(dst)){
        nfl_insert(grid->open_list, dst->idx, g, calc_h(dst, grid->eb));
    }
#ifdef DEBUG
    int h = calc_h(dst, grid->eb);
    int f = g + h;
    DBGprint("=========open (%d %d) g:%d h:%d f:%d prev:(%d, %d)==========\n", dst->col, dst->row, g, h, f, src?src->col:-1, src?src->row:-1);
    //hg_dump(grid);
#endif
}

static HexBlock* get_block_by_offset(HexGrid* grid, int col, int row) {
    col = min_int(grid->w - 1, max_int(0, col));
    row = min_int(grid->h - 1, max_int(0, row));
    return grid->blocks[col + row * grid->w];
}

static int oddr_directions[2][6][2] = {
    {{-1, 0}, {-1, -1}, {0, -1}, {1, 0}, {0, 1}, {-1, 1}},
    {{-1, 0}, {0, -1}, {1, -1}, {1, 0}, {1, 1}, {0, 1}},
};

static HexBlock* get_neighbor(HexGrid* grid, HexBlock* block, int dir) {
    int parity = block->row & 1;
    int* arr = oddr_directions[parity][dir];
    int col = block->col + arr[0];
    int row = block->row + arr[1];
    //printf("get_neighbor, block:(%d %d), dir:%d, (%d, %d)\n", block->col, block->row, dir, col, row);
    if(is_in_grid(grid, col, row)) {
        return grid->blocks[col + row * grid->w];
    } else {
        return NULL;
    }
}

static inline int unwalkable(HexBlock* block, int camp) {
    return block == NULL || (block->obstacle != 0 && block->obstacle != camp);
}

static void set_area(HexGrid* grid, HexBlock* block, int area) {
    block->area = area;
    for(int dir = 0; dir < NO_DIRECTION; ++dir) {
        HexBlock* neighbor = get_neighbor(grid, block, dir);
        if(neighbor!=NULL && neighbor->area == 0 && !unwalkable(neighbor, DEFAULT_CAMP)) {
            set_area(grid, neighbor, area);
        }
    }
}

static void update_area(HexGrid* grid) {
    for(int i = 0; i < grid->w * grid->h; ++i) {
        grid->blocks[i]->area = 0;
    }

    int area = 1;;
    for(int i = 0; i < grid->w * grid->h; ++i) {
        HexBlock* block = grid->blocks[i];
        if(!unwalkable(block, DEFAULT_CAMP) && block->area == 0){
            set_area(grid, block, area++);
        }
    }
}

void hg_init() {
    path = (IntList*)malloc(sizeof(IntList));
    il_init(path, 1);
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
        block->x = block->col - (block->row - ( block->row & 1)) / 2;
        block->z = block->row;
        block->y = -block->x - block->z;
        block->prev_idx = -1;
        block->dirs = 0;
        block->isopen = 0;
        grid->blocks[i] = block;
    }

    grid->w = w;
    grid->h = h;
    grid->sb = NULL;
    grid->eb = NULL;

    grid->open_list = (NodeFreeList *)malloc(sizeof(NodeFreeList));
    nfl_init(grid->open_list);

    grid->dirty_list = (IntList*)malloc(sizeof(IntList));
    il_init(grid->dirty_list, 1);

    DBGprint("create grid(w:%d,h:%d)\n", w, h);
}

void hg_destroy(HexGrid* grid) {
    for(int i = 0; i < grid->w * grid->h; ++i) {
        free(grid->blocks[i]);
    }
    free(grid->blocks);

    nfl_destroy(grid->open_list);
    free(grid->open_list);

    il_destroy(grid->dirty_list);
    free(grid->dirty_list);
}

void hg_set_obstacle(HexGrid* grid, int x, int y, int obstacle) {
    DBGprint("set (%d, %d) = %d\n", x, y, obstacle);
    HexBlock* block = get_block_by_offset(grid, x, y);
    block->obstacle = obstacle;
    update_area(grid);
}

static inline int dir_unwalkable(HexGrid* grid, HexBlock* block, int dir, int camp) {
    return unwalkable(get_neighbor(grid, block, dir), camp);
}

static int FORCE_DIRS[][4] = {
    {DIR_NW, DIR_NE, DIR_SW, DIR_SE}, //DIR_W  0
    {DIR_NE, DIR_E,  DIR_W,  DIR_SW}, //DIR_NW 1
    {DIR_NW, DIR_W,  DIR_E,  DIR_SE}, //DIR_NE 2
    {DIR_NE, DIR_NW, DIR_SE, DIR_SW}, //DIR_E  3
    {DIR_E,  DIR_NE, DIR_SW, DIR_W }, //DIR_SE 4
    {DIR_SE, DIR_E,  DIR_W,  DIR_NW}, //DIR_SW 5
};

static int find_forced_neighbor(HexGrid* grid, HexBlock* block, int dir, int camp) {
    int* arr = FORCE_DIRS[dir];
    for(int i = 0; i<4; i+=2) {
        if(!dir_unwalkable(grid, block, arr[i], camp) && dir_unwalkable(grid, block, arr[i+1], camp)) {
            return 1;
        }
    }
    return 0;
}

static int find_jump_point(HexGrid* grid, HexBlock* src, int dir, int g, int camp) {
    int found = 0;
    HexBlock* next = get_neighbor(grid, src, dir);
    while(next != NULL) {
        g++;
        if(is_close(next) || is_open(next) || unwalkable(next, camp)) {
            return found;
        }
        if(next == grid->eb) {
            //printf("(%d %d)found (%d %d) end, dir:%d\n", src->col, src->row, next->col, next->row, dir);
            add_to_open_list(grid, src, next, g);
            return 1;
        }
        if(find_forced_neighbor(grid, next, dir, camp)) {
            //printf("(%d %d)found (%d %d) forced, dir:%d\n", src->col, src->row, next->col, next->row, dir);
            add_to_open_list(grid, src, next, g);
            found = 1;
        }
        next = get_neighbor(grid, next, dir);
    }
    return found;
}

static void reset_grid(HexGrid* grid) {
    IntList* dirty_list = grid->dirty_list;
    for(int i = 0; i < dirty_list->num; ++i) {
        HexBlock* block = grid->blocks[il_get(dirty_list, i, 0)];
        block->dirs = 0;
        block->prev_idx = -1;;
        block->isopen = 0;
    }
    il_clear(dirty_list);
    nfl_clear(grid->open_list);
    grid->sb = NULL;
    grid->eb = NULL;
}

IntList* hg_pathfinding(HexGrid* grid, int c1, int r1, int c2, int r2, int camp) {
    //DBGprint("pathfinding (%d, %d) => (%d, %d)\n", x1, y1, x2, y2);
    HexBlock* start = get_block_by_offset(grid, c2, r2);
    HexBlock* end = get_block_by_offset(grid, c1, r1);

    il_clear(path);
    if(start->area != end->area || unwalkable(start, camp) || unwalkable(end, camp)) {
        DBGprint("no way, start:%d, end:%d, area:%d\n",
            unwalkable(start, camp), unwalkable(end, camp), start->area != end->area);
        return path;
    }
    if(start == end) {
        add_to_path(start);
        return path;
    }

    grid->sb = start; // 倒过来寻路
    grid->eb = end;

    NodeFreeList* open_list = grid->open_list;
    add_to_open_list(grid, NULL, start, 0);

    while(!nfl_is_empty(open_list)) {
        Node* node = nfl_head(open_list);
        HexBlock* block = grid->blocks[node->idx];
        if(block == end) {
            break;
        }
        //printf("current: (%d %d), g:%d, h:%d, f:%d\n", node->idx % grid->w, node->idx / grid->w, node->g, node->h, node->f);
        for(int dir = 0; dir < NO_DIRECTION; ++dir) {
            if(!in_dirs(block->dirs, dir)) {
                add_dir(grid, block, dir);
                if(is_close(block)) {
                    nfl_remove(grid->open_list, node);
                    //printf("## close block(%d %d)\n", block->col, block->row);
                }
                if(find_jump_point(grid, block, dir, node->g, camp)){
                    continue;
                }
            }
        }

        for(int dir = 0; dir < NO_DIRECTION; ++dir) {
            HexBlock* neighbor = get_neighbor(grid, block, dir);
            if(neighbor != NULL)
                //printf("search_around:(%d %d) => (%d %d), dir:%d, g:%d\n",
                //        block->col, block->row, neighbor->col, neighbor->row, dir, node->g+1);
            if(!unwalkable(neighbor, camp) && !is_close(neighbor) && !is_open(neighbor)) {
                add_to_open_list(grid, block, neighbor, node->g+1);
            }
        }

        if(grid->eb->prev_idx >= 0) {
            break;
        }
    }

    //hg_dump(grid);

    if(end->prev_idx >= 0) {
        int idx = end->idx;
        while(idx >= 0) {
            HexBlock* cur = grid->blocks[idx];
            add_to_path(cur);
            if(idx == cur->prev_idx) {
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
    for(int y = 0; y < h; ++ y){
        if(y%2!=0)
            printf(" ");
        for(int x = 0; x < w; ++x){
            HexBlock* block = grid->blocks[y * grid->w + x];
            if(block->obstacle != 0)
                printf("X ");
            else
                printf("%d ", block->area);
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
