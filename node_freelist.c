#include "node_freelist.h"
#include <stdlib.h>
#include <string.h>

NodeFreeList* nfl_create() {
    NodeFreeList* fl = (NodeFreeList *)malloc(sizeof(NodeFreeList));
    fl->data = fl->fixed;
    fl->fill_num = 0;
    fl->cap = fixed_cap;
    fl->free_element = -1;
    fl->head = NULL;
    fl->tail = NULL;
    return fl;
}

void nfl_destroy(NodeFreeList* fl) {
    // 如果有堆分配记得释放
    if (fl->data != fl->fixed) {
        free(fl->data);
    }
}

// 从小到大排序插入
int nfl_insert(NodeFreeList* fl, int pos, int g, int h) {
    int index;
    if (fl->free_element != -1) {
        index = fl->free_element;
        fl->free_element = fl->data[index].next;
    } else {
        if (fl->fill_num >= fl->cap) {
            nfl_reserve(fl, fl->cap * 2);
        }
        index = fl->fill_num++;
    }
    int f = g + h;
    Node* node = &fl->data[index];
    node->pos = pos;
    node->h = h;
    node->g = g;
    node->f = f;
    node->cur = index;

    // 插入排序
    if(!fl->head) {
        node->next = fl->free_element;
        fl->head = node;
        fl->tail = node;
    } else {
        Node* prev = NULL;
        Node* cur = fl->head;
        while(cur && cur->f < node->f) {
            if(cur == fl->tail) {
                break;
            }
            prev = cur;
            int idx = cur->next;
            if(idx == -1) {
                break;
            }
            cur = &fl->data[cur->next];
        }
        if(!prev) {
            // 插入到表头
            node->next = fl->head->cur;
            fl->head = node;
        }
        node->next = prev->next;
        prev->next = index;
    }

    return index;
}

void nfl_remove(NodeFreeList* fl, int n) {
    if (n >= 0 && n < fl->fill_num) {
        Node* cur = &fl->data[n];
        cur->next = fl->free_element;
        fl->free_element = n;
    }
}

Node* nfl_get(NodeFreeList* fl, int n) {
    if (n >= 0 && n < fl->fill_num) {
        return &fl->data[n];
    }
    return NULL;
}

void nfl_reserve(NodeFreeList* fl, int n) {
    if (n > fl->cap) {
        if (fl->cap == fixed_cap) {
            fl->data = (Node *)malloc(n * sizeof(Node));
            memcpy(fl->data, fl->fixed, sizeof(fl->fixed));
        } else {
            fl->data = realloc(fl->data, n * sizeof(Node));
        }
        fl->cap = n;
    }
}
