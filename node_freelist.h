#ifndef NODE_FREE_LIST_H
#define NODE_FREE_LIST_H

#ifdef __cplusplus
#define NODE_FL_FUNC extern "C"
#else
#define NODE_FL_FUNC
#endif

typedef struct Node Node;
typedef struct NodeFreeList NodeFreeList;

struct Node {
    // 存储下一个节点位置
    int next;

    // 存储当前节点的句柄
    int handle;

    // 存储节点的位置
    float x, y;
};

enum { fixed_cap = 128 };

struct NodeFreeList {
    // 预分配的节点 buffer
    Node fixed[fixed_cap];

    // 节点列表指针，初始化指向 fixed，后续扩容了需要把 fixed 拷贝进新内存
    Node* data;

    // 存储节点数量
    int num;

    // 上限，初始化为 fixed_cap
    int cap;

    // 存储 freelist 空闲位置
    int free_element;
};

// 创建
NODE_FL_FUNC NodeFreeList* nfl_create();

// 销毁，会释放内存
NODE_FL_FUNC void nfl_destroy(NodeFreeList* fl);

// 插入一个节点，这里的 next 是通过 grid 获取到当前的
NODE_FL_FUNC int nfl_insert(NodeFreeList* fl, int handle, float x, float y, int next);

// 移除
NODE_FL_FUNC void nfl_remove(NodeFreeList* fl, int n);

// 返回第 n 个节点
NODE_FL_FUNC Node* nfl_get(NodeFreeList* fl, int n);

// 主动扩容，保证能容下 n 个元素，在 grid 模块的优化接口里调用
NODE_FL_FUNC void nfl_reserve(NodeFreeList* fl, int n);

#endif
