local hex_grid = require "hex_grid"
local map = require "test.sample_map"

local function to_idx(col, row)
    return row * map.w + col
end

local function to_offset(idx)
    return idx % map.w, idx // map.w
end

local hg = hex_grid.new(map.w, map.h)
local list = {}
for _, v in pairs(map.blocks) do
    if v.obstacle then
        list[#list+1] = {to_idx(v.col, v.row), -1}
    end
end

hg:set_obstacles(list)
hg:set_obstacle(to_idx(1, 0), 1)
hg:dump()
print("obstacle(1, 1)", hg:get_obstacle(to_idx(1, 1)))
print("obstacle(0, 0)", hg:get_obstacle(to_idx(0, 0)))

--[[
if hg.test_open_dump then
    hg:test_open_insert(1, 1, 1)
    hg:test_open_dump()
    hg:test_open_insert(2, 2, 1)
    hg:test_open_dump()
    hg:test_open_insert(3, 0, 1)
    hg:test_open_dump()
    hg:test_open_insert(4, 5, 1)
    hg:test_open_dump()
    hg:test_open_pop()
    hg:test_open_dump()
    hg:test_open_insert(5, 3, 1)
    hg:test_open_dump()
    hg:test_open_pop()
    hg:test_open_pop()
    hg:test_open_pop()
    hg:test_open_pop()
    hg:test_open_insert(6, 9, 1)
    hg:test_open_dump()
end
]]

local function find(x1, y1, x2, y2, camp)
    local path = hg:pathfinding(to_idx(x1, y1), to_idx(x2, y2), camp)
    print(string.format("----------- find path: (%d, %d) => (%d, %d) camp:%s ------------", x1, y1, x2, y2, camp))
    for _, v in pairs(path or {}) do
        print(string.format("(%s, %s)", to_offset(v)))
    end
    print("----------------------------------------------------")
end

find(0, 0, 2, 2, 3)
find(0, 0, 5, 3)
find(5, 3, 0, 0)
find(5, 3, 2, 3)
find(0, 0, 1, 1)
find(0, 0, 0, 0)
find(0, 0, 2, 2, 1)
find(0, 0, 1, 0)
