#ifndef HEX_GRID_H
#define HEX_GRID_H

#include "intlist.h"

#ifdef __cplusplus
#define HEX_GRID_FUNC extern "C"
#else
#define HEX_GRID_FUNC
#endif

typedef struct HexGrid HexGrid;

// 整个网格
struct HexGrid {
    // 所有格子
    int* cells;

    int w, h;
};

// 创建地图网格
HEX_GRID_FUNC void hg_create(HexGrid* grid, int w, int h);

// 销毁
HEX_GRID_FUNC void hg_destroy(HexGrid* grid);

// 设置障碍
// camp ^ obstacles > 0 表示不可通行
// obstacles = 0xfffff 则是全部单位不可通行
HEX_GRID_FUNC void hg_set(HexGrid* grid, int x, int y, int obstacles);

HEX_GRID_FUNC void hg_pathfinding(HexGrid* grid, int x1, int y1, int x2, int y2);

#endif
