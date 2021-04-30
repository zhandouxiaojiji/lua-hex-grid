#ifndef INT_LIST_H
#define INT_LIST_H

#ifdef __cplusplus
#define IL_FUNC extern "C"
#else
#define IL_FUNC
#endif

typedef struct IntList IntList;
// 默认先分配 128 个 int 空间
enum { il_fixed_cap = 128 };

struct IntList {
    // 预先分配的固定空间，防止小数量元素频繁申请内存，超过此空间才去申请内存
    // 申请了新内存把此空间内存拷贝进新内存
    int fixed[il_fixed_cap];

    // 指向当前 buffer 的指针，初始化指向 fixed 数组，后续申请了新内存指向新内存
    int* data;

    // 每一个元素的数据域个数，相当于一个元素需要占据多少个 int
    int num_fields;

    // 元素数量
    int num;

    // 最大长度，初始化为 il_fixed_cap
    int cap;

    // 记录 freelist 空闲位置，移除一个元素就会富裕出一个空闲位置,-1 表示没有空闲位置了
    int free_element;
};

// 这个库可以工作在三种模式下，一种是 array，一种是 stack，一种是 freelist
// stack 模式跟 freelist 模式之间不能混淆
// array 模式的函数跟 stack 、 freeslit 模式可以兼容
// il_size 函数在 freelist 模式下不能获得正确的数量

// 创建，指定每个元素所占用的数域个数
IL_FUNC void il_create(IntList* il, int num_fields);

// 销毁，如果动态扩容了，会释放掉堆内存
IL_FUNC void il_destroy(IntList* il);

// ---------------------------------------------------------------------------------
// array 模式
// ---------------------------------------------------------------------------------
// 返回元素数量
IL_FUNC int il_size(const IntList* il);

// 返回第 n 个元素的第 field 个数域，field 从 0 开始
IL_FUNC int il_get(const IntList* il, int n, int field);

// 设置第 n 个元素的第 field 个数域值为 val，field 从 0 开始
IL_FUNC void il_set(IntList* il, int n, int field, int val);

// 清空这个数组，只是清空，并不会释放内存，cap 是不会缩小的
IL_FUNC void il_clear(IntList* il);

// ---------------------------------------------------------------------------------
// stack 模式
// ---------------------------------------------------------------------------------
// 往尾部压入一个元素，可能触发内存分配，返回占据的位置
// 需要注意的是此接口返回被占用的位置，后续还需要使用 il_set 设置具体的值
IL_FUNC int il_push_back(IntList* il);

// 移除尾部的元素
IL_FUNC void il_pop_back(IntList* il);

// ---------------------------------------------------------------------------------
// freelist 模式
// ---------------------------------------------------------------------------------
// 往 freelist 里插入一个元素，如果没有空闲的位置，会调用 il_push_back 往尾部插入，返回占据的位置
// 跟 il_push_back 接口一样，后续还需要使用 il_set 设置具体的值
IL_FUNC int il_insert(IntList* il);

// 移除第 n 位置的元素，会产生一个 free 空洞
IL_FUNC void il_erase(IntList* il, int n);

#endif
