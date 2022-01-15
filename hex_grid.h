#ifndef HEX_GRID_H
#define HEX_GRID_H

#include <stddef.h>
#include "intlist.h"
#include "node_freelist.h"

#ifdef __cplusplus
#define HEX_GRID_FUNC extern "C"
#else
#define HEX_GRID_FUNC
#endif

#define FLAG_INIT 0
#define FLAG_OPEN 1
#define FLAG_CLOSE 2

#define OBS_NONE 0 // 没有阻挡
#define OBS_BUILDING 1 // 建筑阻挡
#define OBS_TERRAIN 2 // 地形阻挡

typedef struct HexGrid HexGrid;
typedef struct HexBlock HexBlock;

struct HexBlock {
    int obstacle; // 障碍标识
    int idx;      // blocks index
    int col;      // offset col
    int row;      // offset row
    int x;        // cube x
    int y;        // cube y
    int z;        // cube z
    int area;     // 按(DEFAULT_OBSTACLE)划分的区域
    int prev_idx; // 上一个跳点
    int gscore;
    int hscore;
    unsigned char flag; // open & close标志
    Node* node;
};

// 整个网格
struct HexGrid {
    // 所有格子
    HexBlock** blocks;

    int w, h;

    HexBlock* sb;
    HexBlock* eb;

    NodeFreeList* open_list;
    IntList* dirty_list;
};


HEX_GRID_FUNC void hg_init();

// 创建地图网格
HEX_GRID_FUNC void hg_create(HexGrid* grid, int w, int h);

// 销毁
HEX_GRID_FUNC void hg_destroy(HexGrid* grid);

// 设置障碍
HEX_GRID_FUNC void hg_set_obstacle(HexGrid* grid, int pos, int obs);

HEX_GRID_FUNC int hg_get_obstacle(HexGrid* grid, int pos);

HEX_GRID_FUNC int hg_get_area(HexGrid* grid, int pos);

HEX_GRID_FUNC void hg_update_area(HexGrid* grid);

HEX_GRID_FUNC IntList* hg_pathfinding(HexGrid* grid, int pos1, int pos2, int ignore_lv);

HEX_GRID_FUNC void hg_dump(HexGrid* grid);

HEX_GRID_FUNC NodeFreeList* hg_get_open_list();
HEX_GRID_FUNC IntList* hg_get_close_list();

#endif
