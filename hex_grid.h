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

#define UNWALKABLE -1
#define IGNORE_OBSTACLE -2

typedef struct HexGrid HexGrid;
typedef struct HexBlock HexBlock;
/*
 * 障碍标识obstacle 说明：
 *  0: 可通行
 *  -1: 地形阻挡(所有单位不可通行，不可作为终点)
 *  大于0: 阵营阻挡(指定阵营单位可通行，仅阻挡寻路，可作为终点)
 */
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
    unsigned char dirs; // 检索方向
    unsigned char isopen;
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
// obstacle: -1 所有单位不可通行
// camp == obstacle || obstacle == 0 可通行
HEX_GRID_FUNC void hg_set_obstacle(HexGrid* grid, int pos, int obs);

HEX_GRID_FUNC int hg_get_obstacle(HexGrid* grid, int pos);

HEX_GRID_FUNC void hg_update_area(HexGrid* grid);

HEX_GRID_FUNC IntList* hg_pathfinding(HexGrid* grid, int pos1, int pos2, int camp);

HEX_GRID_FUNC void hg_dump(HexGrid* grid);

HEX_GRID_FUNC NodeFreeList* hg_get_open_list();
HEX_GRID_FUNC IntList* hg_get_close_list();

#endif
