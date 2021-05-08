# 六边形网格地图

目前只支持 "odd-r" 一种形态的六边形网格:

![](screenshots/odd-r.png)

参考文档：https://www.redblobgames.com/grids/hexagons/

## 测试
```sh
make # 编译并运行test.lua
make debug # 编译debug模式打印详细日志
```

## 地图数据模板
```lua
{
    id = 101,   -- RTS地图唯一id
    w = 10,     -- 地图宽
    h = 5,      -- 地图高
    blocks = {  -- 网格配置
        {col = 1, row = 0, obstacle = true, area = 3, terrain = 101},
        {col = 1, row = 1, obstacle = true, area = 3, terrain = 102},
        {col = 1, row = 2, obstacle = true, area = 3, terrain = 103},
        {col = 1, row = 3, obstacle = true, area = 3, terrain = 104},
        {col = 5, row = 3, area = 2, terrain = 105},
        {col = 0, row = 0, area = 1, terrain = 105},
    },
}
```
block字段
+ col 纵坐标
+ row 横坐标
+ obstacle 有障碍不可通行
+ area 区域类型
+ terrain 地形(草地山川之类的)
>blocks暂时用col，row配置有内容的格子，没有配置的格子默认可走，等编辑器完成后则不再需要col和row，用数组配满所有格子

## 类型定义
```lua
-- 区域类型
M.AREA_TYPE = {
    CROSSING  = 1, -- 路口
    TENT      = 2, -- 营帐区
    BLANK     = 3, -- 空白
}

-- 建筑类型
M.BUILDING_TYPE = {
    TENT         = 1, -- 营帐
    MINE         = 2, -- 矿
    REJECT_HORSE = 3, -- 拒马
    ARROW_TOWER  = 4, -- 箭塔
}

-- 单位类型
M.ELEM_TYPE = {
    TEAM    = 1, -- 队伍
    MONSTER = 2, -- 怪物
}
```