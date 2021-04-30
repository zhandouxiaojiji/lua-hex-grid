#include "node_freelist.h"
#include <stdlib.h>
#include <string.h>

NodeFreeList* nfl_create() {
    NodeFreeList* fl = (NodeFreeList *)malloc(sizeof(NodeFreeList));
    fl->data = fl->fixed;
    fl->num = 0;
    fl->cap = fixed_cap;
    fl->free_element = -1;
    return fl;
}

void nfl_destroy(NodeFreeList* fl) {
    // 如果有堆分配记得释放
    if (fl->data != fl->fixed) {
        free(fl->data);
    }
}

int nfl_insert(NodeFreeList* fl, int handle, float x, float y, int next) {
    int index;
    if (fl->free_element != -1) {
        index = fl->free_element;
        fl->free_element = fl->data[index].next;
    } else {
        if (fl->num >= fl->cap) {
            nfl_reserve(fl, fl->cap * 2);
        }
        index = fl->num++;
    }
    fl->data[index].handle = handle;
    fl->data[index].x = x;
    fl->data[index].y = y;
    fl->data[index].next = next;
    return index;
}

void nfl_remove(NodeFreeList* fl, int n) {
    if (n >= 0 && n < fl->num) {
        fl->data[n].next = fl->free_element;
        fl->free_element = n;
    }
}

Node* nfl_get(NodeFreeList* fl, int n) {
    if (n >= 0 && n < fl->num) {
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
