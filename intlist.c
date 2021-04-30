#include "intlist.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

void il_create(IntList* il, int num_fields) {
    il->data = il->fixed;
    il->num = 0;
    il->cap = il_fixed_cap;
    il->num_fields = num_fields;
    il->free_element = -1;
}

void il_destroy(IntList* il) {
    // 如果有堆分配记得释放
    if (il->data != il->fixed) {
        free(il->data);
    }
}

void il_clear(IntList* il) {
    il->num = 0;
    il->free_element = -1;
}

int il_size(const IntList* il) {
    return il->num;
}

int il_get(const IntList* il, int n, int field) {
    assert(n >= 0 && n < il->num);
    return il->data[n*il->num_fields + field];
}

void il_set(IntList* il, int n, int field, int val) {
    assert(n >= 0 && n < il->num);
    il->data[n*il->num_fields + field] = val;
}

int il_push_back(IntList* il) {
    const int new_pos = (il->num+1) * il->num_fields;

    // 如果 list 满了，重新分配 buffer
    if (new_pos > il->cap) {
        // 空间倍增
        const int new_cap = new_pos * 2;

        // 如果当前最大长度跟固定长度相等，证明是首次扩容
        // 需要将固定长度 buffer 内的元素拷贝进新分配的 buffer
        if (il->cap == il_fixed_cap) {
            il->data = malloc(new_cap * sizeof(*il->data));
            memcpy(il->data, il->fixed, sizeof(il->fixed));
        } else {
            // 否则直接扩大 buffer 的大小
            il->data = realloc(il->data, new_cap * sizeof(*il->data));
        }
        il->cap = new_cap;
    }
    return il->num++;
}

void il_pop_back(IntList* il) {
    assert(il->num > 0);
    --il->num;
}

int il_insert(IntList* il) {
    // 如果 freelist 里还有空间，直接用
    if (il->free_element != -1) {
        const int index = il->free_element;
        const int pos = index * il->num_fields;

        il->free_element = il->data[pos];

        return index;
    }
    // 否则就往后插，可能触发重新分配内存
    return il_push_back(il);
}

void il_erase(IntList* il, int n) {
    // 移除一个元素会空出一个 freelist 节点
    const int pos = n * il->num_fields;
    il->data[pos] = il->free_element;
    il->free_element = n;
}
