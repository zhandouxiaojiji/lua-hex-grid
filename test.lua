
local hex_grid = require "hex_grid"

local data = [[
* * * * * *
 * * * * * *
* @ @ @ * *
 * * * @ * *
* * * * * *
]]

local map = {
    w = 10,
    h = 5,
}

local hg = hex_grid.new(10, 5)
hg:set(1, 1, 100)
hg:dump()

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


