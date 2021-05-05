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

#define DEFAULT_OBSTACLE -1
#define DEFAULT_CAMP -2

typedef struct HexGrid HexGrid;

// 整个网格
struct HexGrid {
    // 所有格子
    int* blocks;

    int w, h;
};


HEX_GRID_FUNC void hg_init();

// 创建地图网格
HEX_GRID_FUNC void hg_create(HexGrid* grid, int w, int h);

// 销毁
HEX_GRID_FUNC void hg_destroy(HexGrid* grid);

// 设置障碍
// obstacle: -1 所有单位不可通行
// camp == obstacle || obstacle == 0 可通行
HEX_GRID_FUNC void hg_set(HexGrid* grid, int x, int y, int obs);

HEX_GRID_FUNC IntList* hg_pathfinding(HexGrid* grid, int x1, int y1, int x2, int y2, int camp);

HEX_GRID_FUNC void hg_dump(HexGrid* grid);

HEX_GRID_FUNC NodeFreeList* hg_get_open_list();
HEX_GRID_FUNC IntList* hg_get_close_list();

#endif
