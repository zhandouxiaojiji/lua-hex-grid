
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
