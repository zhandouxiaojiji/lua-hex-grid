local hex_grid = require "hex_grid"
local map = require "test.sample_map"

local hg = hex_grid.new(map.w, map.h)
for _, v in pairs(map.blocks) do
    if v.obstacle then
        hg:set(v.col, v.row, -1)
    end
end
hg:dump()

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
    local path = hg:pathfinding(x1, y1, x2, y2, camp)
    print(string.format("----------- find path: (%d, %d) => (%d, %d) ------------", x1, y1, x2, y2))
    for _, v in pairs(path or {}) do
        print(string.format("(%s, %s)", v[1], v[2]))
    end
    print("----------------------------------------------------")
end

find(0, 0, 2, 2)
find(0, 0, 5, 3)
find(5, 3, 0, 0)
find(5, 3, 2, 3)
find(0, 0, 1, 1)
find(0, 0, 0, 0)
