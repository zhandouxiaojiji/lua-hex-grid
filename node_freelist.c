#include "node_freelist.h"
#include <stdlib.h>
#include <string.h>

void nfl_init(NodeFreeList* fl) {
    fl->data = fl->fixed;
    fl->fill_num = 0;
    fl->cap = fixed_cap;
    fl->free_element = -1;
    fl->head = -1;
    fl->tail = -1;
}

void nfl_destroy(NodeFreeList* fl) {
    if (fl->data != fl->fixed) {
        free(fl->data);
    }
}

void nfl_clear(NodeFreeList* fl) {
    for(int i = 0; i< fl->fill_num; ++i) {
        fl->data[i].next = -1;
    }
    fl->head = -1;
    fl->tail = -1;
    fl->free_element = -1;
    fl->fill_num = 0;
}

int nfl_is_empty(NodeFreeList* fl) {
    return fl->head == -1;
}

Node* nfl_head(NodeFreeList* fl) {
    if(fl->head < 0) {
        return NULL;
    }
    return &fl->data[fl->head];
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
    if(fl->head < 0) {
        node->next = fl->free_element;
        fl->head = index;
        fl->tail = index;
    } else {
        int prev = -1;
        int cur = fl->head;
        //while(f > fl->data[cur].f || (f == fl->data[cur].f && h > fl->data[cur].h)) {
        while(f > fl->data[cur].f) {
            prev = cur;
            if(cur == fl->tail) {
                break;
            }
            cur = fl->data[cur].next;
        }
        if(prev >= 0) {
            if(prev == fl->tail) {
                fl->tail = index;
            }
            node->next = fl->data[prev].next;
            fl->data[prev].next = index;
        } else{
            // 插入到表头
            node->next = fl->head;
            fl->head = index;
        }
        //printf("### prev:%d, cur:%d, index:%d\n", prev, cur, index);
    }

    return index;
}

Node* nfl_pop(NodeFreeList* fl) {
    if(fl->head < 0) {
        return NULL;
    }
    Node* node = nfl_head(fl);
    if(fl->head == fl->tail) {
        fl->head = -1;
        fl->tail = -1;
    } else {
        fl->head = node->next;
    }
    node->next = fl->free_element;
    fl->free_element = node->cur;
    return node;
}

void nfl_remove(NodeFreeList* fl, Node* dst) {
    Node* node = nfl_head(fl);
    // 删除head
    if(node == dst){
        nfl_pop(fl);
        return;
    }
    int prev = -1;
    while(node) {
        if(node->cur == fl->tail) {
            // 删除tail
            if(prev >= 0) {
                fl->tail = prev;
            }
            break;
        }
        if(node == dst) {
            fl->data[prev].next = dst->next;
            break;
        }
        prev = node->cur;
        if(node->next < 0)
            break;
        node = &fl->data[node->next];
    }

    dst->next = fl->free_element;
    fl->free_element = dst->cur;

}

Node* nfl_get_top(NodeFreeList* fl) {
    return &fl->data[fl->head];
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
