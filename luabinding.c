#if __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "hex_grid.h"

#ifdef DEBUG
#include <stdio.h>
#endif

#ifndef LUA_LIB_API
#define LUA_LIB_API extern
#endif

static IntList out_put;

#define MT_NAME ("_hg_metatable")

static int
gd_set(lua_State *L) {
    HexGrid *grid = luaL_checkudata(L, 1, MT_NAME);
    int x = luaL_checkinteger(L, 2);
    int y = luaL_checkinteger(L, 3);
    int obstacles = luaL_checkinteger(L, 4);
    hg_set(grid, x, y, obstacles);
    return 0;
}

static int
gd_pathfinding(lua_State *L) {
    HexGrid *grid = luaL_checkudata(L, 1, MT_NAME);
    int x1 = luaL_checkinteger(L, 2);
    int y1 = luaL_checkinteger(L, 3);
    int x2 = luaL_checkinteger(L, 4);
    int y2 = luaL_checkinteger(L, 5);
    hg_pathfinding(grid, x1, y1, x2, y2);
    return 0;
}

static int
gd_dump(lua_State *L) {
    HexGrid *grid = luaL_checkudata(L, 1, MT_NAME);
    hg_dump(grid);
    return 0;
}

static int
gc(lua_State *L) {
    HexGrid *grid = luaL_checkudata(L, 1, MT_NAME);
    hg_destroy(grid);
    return 0;
}

#ifdef DEBUG
static int
test_open_insert(lua_State* L) {
    HexGrid *grid = luaL_checkudata(L, 1, MT_NAME);
    int pos = luaL_checkinteger(L, 2);
    int g = luaL_checkinteger(L, 3);
    int h = luaL_checkinteger(L, 4);
    nfl_insert(grid->open_list, pos, g, h);
    return 0;
}

static int
test_open_pop(lua_State* L) {
    HexGrid *grid = luaL_checkudata(L, 1, MT_NAME);
    nfl_pop(grid->open_list);
    return 0;
}

static int
test_open_dump(lua_State* L) {
    HexGrid* grid = luaL_checkudata(L, 1, MT_NAME);
    printf("---------------dump open_list--------------------\n");
    NodeFreeList* list = grid->open_list;
    if(list->head >= 0) {
        Node* node = &list->data[list->head];
        int count = 0;
        while(node) {
            printf("(%d:%d->%d) ", node->cur, node->f, node->next);
            if(node->cur == list->tail)
                break;
            node = &list->data[node->next];
            if(count++ >1000){
                printf("may endless");
                break;
            }
        }
        printf("\n");
    } else {
        printf("list empty\n");
    }
    printf("head:%d, tail:%d\n", list->head, list->tail);
    printf("-------------------------------------------------\n");
    return 0;
}

#endif

static int
lmetatable(lua_State *L) {
    if (luaL_newmetatable(L, MT_NAME)) {
        luaL_Reg l[] = {
            { "set", gd_set },
            { "pathfinding", gd_pathfinding },
            { "dump", gd_dump },
#ifdef DEBUG
            { "test_open_insert", test_open_insert },
            { "test_open_pop", test_open_pop },
            { "test_open_dump", test_open_dump },
#endif
            { NULL, NULL }
        };
        luaL_newlib(L, l);
        lua_setfield(L, -2, "__index");

        lua_pushcfunction(L, gc);
        lua_setfield(L, -2, "__gc");
    }
    return 1;
}

static int
lnew(lua_State *L) {
    int w = luaL_checkinteger(L, 1);
    int h = luaL_checkinteger(L, 2);
    lua_assert(w > 0 && h > 0);
#if LUA_VERSION_NUM == 504
    HexGrid *grid = lua_newuserdatauv(L, sizeof(HexGrid), 0);
#else
    HexGrid *grid = lua_newuserdata(L, sizeof(HexGrid));
#endif
    hg_create(grid, w, h);
    lmetatable(L);
    lua_setmetatable(L, -2);
    return 1;
}

LUA_LIB_API int
luaopen_hex_grid(lua_State* L) {
    il_create(&out_put, 1);
    luaL_checkversion(L);
    luaL_Reg l[] = {
        { "new", lnew },
        { NULL, NULL },
    };
    luaL_newlib(L, l);
    return 1;
}

#if __cplusplus
}
#endif
