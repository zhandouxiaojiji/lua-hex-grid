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
    // 在freelist中的位置
    int cur;
    // 下一个节点位置
    int next;

    // 存储节点的位置
    int idx;

    int f;
    int g;
    int h;
};

enum { fixed_cap = 128 };

struct NodeFreeList {
    // 预分配的节点 buffer
    Node fixed[fixed_cap];

    // 节点列表指针，初始化指向 fixed，后续扩容了需要把 fixed 拷贝进新内存
    Node* data;

    int head;
    int tail;

    // 存储节点数量
    int fill_num;

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
NODE_FL_FUNC int nfl_insert(NodeFreeList* fl, int idx, int g, int h);

// 移除
NODE_FL_FUNC Node* nfl_pop(NodeFreeList* fl);
NODE_FL_FUNC void nfl_remove(NodeFreeList* fl, Node* dst);

NODE_FL_FUNC Node* nfl_head(NodeFreeList* fl);

NODE_FL_FUNC void nfl_clear(NodeFreeList* fl);

// 主动扩容，保证能容下 n 个元素，在 grid 模块的优化接口里调用
NODE_FL_FUNC void nfl_reserve(NodeFreeList* fl, int n);

NODE_FL_FUNC int nfl_is_empty(NodeFreeList* fl);

#endif
