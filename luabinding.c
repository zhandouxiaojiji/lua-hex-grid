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

#define MT_NAME ("_hg_metatable")

static int
lhg_set_obstacle(lua_State* L) {
    HexGrid *grid = luaL_checkudata(L, 1, MT_NAME);
    int pos = luaL_checkinteger(L, 2);
    int obstacle = UNWALKABLE;
    if(lua_isinteger(L, 3)) {
        obstacle = luaL_checkinteger(L, 3);
    }
    hg_set_obstacle(grid, pos, obstacle);
    hg_update_area(grid);
    return 0;
}

static int
lhg_set_obstacles(lua_State* L) {
    HexGrid *grid = luaL_checkudata(L, 1, MT_NAME);
    luaL_checktype(L, 2, LUA_TTABLE);
    lua_settop(L, 2);

    int i = 1;
    while (lua_geti(L, -1, i) == LUA_TTABLE) {
        lua_geti(L, -1, 1);
        int pos = lua_tointeger(L, -1);
        lua_geti(L, -2, 2);
        int obstacle = lua_tointeger(L, -1);
        hg_set_obstacle(grid, pos, obstacle);
        lua_pop(L, 3);
        i++;
    }
    hg_update_area(grid);
    return 0;
}

static int
lhg_get_obstacle(lua_State* L) {
    HexGrid *grid = luaL_checkudata(L, 1, MT_NAME);
    int pos = luaL_checkinteger(L, 2);
    lua_pushinteger(L, hg_get_obstacle(grid, pos));
    return 1;
}

static int
lhg_pathfinding(lua_State* L) {
    HexGrid *grid = luaL_checkudata(L, 1, MT_NAME);
    int pos1 = luaL_checkinteger(L, 2);
    int pos2 = luaL_checkinteger(L, 3);
    int camp = IGNORE_OBSTACLE;
    if(lua_isinteger(L, 4)) {
        camp = luaL_checkinteger(L, 4);
        if(camp <= 0) {
            luaL_error(L, "camp:%d must large than 0", camp);
            return 0;
        }
    }
    IntList* path = hg_pathfinding(grid, pos1, pos2, camp);

    lua_newtable(L);
    for (int i = 0; i < il_size(path); ++i) {
        int idx = il_get(path, i, 0);
        lua_pushinteger(L, idx);
        lua_rawseti(L, -2, i+1);
    }
    return 1;
}

static int
lhg_dump(lua_State* L) {
    HexGrid *grid = luaL_checkudata(L, 1, MT_NAME);
    hg_dump(grid);
    return 0;
}

static int
gc(lua_State* L) {
    HexGrid *grid = luaL_checkudata(L, 1, MT_NAME);
    hg_destroy(grid);
    return 0;
}

#ifdef DEBUG
static int
test_open_insert(lua_State* L) {
    HexGrid *grid = luaL_checkudata(L, 1, MT_NAME);
    int idx = luaL_checkinteger(L, 2);
    int g = luaL_checkinteger(L, 3);
    int h = luaL_checkinteger(L, 4);
    nfl_insert(hg_get_open_list(grid), idx, g, h);
    return 0;
}

static int
test_open_pop(lua_State* L) {
    HexGrid *grid = luaL_checkudata(L, 1, MT_NAME);
    nfl_pop(hg_get_open_list(grid));
    return 0;
}

static int
test_open_dump(lua_State* L) {
    HexGrid* grid = luaL_checkudata(L, 1, MT_NAME);
    printf("---------------dump open_list--------------------\n");
    NodeFreeList* list = hg_get_open_list(grid);
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
            { "set_obstacle", lhg_set_obstacle },
            { "set_obstacles", lhg_set_obstacles },
            { "get_obstacle", lhg_get_obstacle },
            { "pathfinding", lhg_pathfinding },
            { "dump", lhg_dump },
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
    hg_init();
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
