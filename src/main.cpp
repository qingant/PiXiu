#include "common/gen.h"
#include "common/List.h"
#include "common/Que.h"
#include <stdio.h>

void t_CritBitTree(void);

void t_gen(void);

void t_List(void);

void t_Que(void);

int main() {
#ifndef NDEBUG
    t_CritBitTree();
    t_gen();
    t_List();
    t_Que();

    printf("\nt_OK\n");
#endif


    return 0;
}

void t_Que(void) {
    int q[] = {0, 0, 0};
    int q_len = 3;
    int q_cursor = 0;

    Que_push(q, 1);
    Que_push(q, 2);
    Que_push(q, 3);
    Que_push(q, 4);
    Que_push(q, 5);

    assert(Que_get(q, 0) == 3);
    assert(Que_get(q, 1) == 4);
    assert(Que_get(q, 2) == 5);
}

$gen(range0_10) {
    // vars
    int i;
    // constructor

    // body
    $emit(int)
            for (i = 0; i < 10; ++i) {
                $yield(i);
            }
    $stop;
};

void t_gen(void) {
    range0_10 gen;
    int rv;
    for (int i = 0; i < 10; ++i) {
        assert(gen(rv));
        assert(rv == i);
    }
    assert(!gen(rv));
}

void t_List(void) {
    int ptrAs(expect);
    List_init(int, list);

    List_append(int, list, 3);
    List_append(int, list, 2);
    List_append(int, list, 1);
    expect = (int[]) {3, 2, 1};
    for (int i = 0; i < list_len; ++i) {
        assert(list[i] == expect[i]);
    }
    assert(list_len == 3 && list_capacity == 4);

    List_del(int, list, 1);
    assert(list[0] == 3 && list[1] == 1);
    List_del(int, list, 1);
    assert(list[0] == 3);
    List_del(int, list, 0);
    assert(list_len == 0);

    List_insert(int, list, 0, 0);
    List_insert(int, list, 0, 1);
    List_insert(int, list, 1, 2);
    expect = (int[]) {1, 2, 0};
    for (int i = 0; i < list_len; ++i) {
        assert(list[i] == expect[i]);
    }
    assert(list_len == 3);

    List_insert(int, list, 3, 3);
    List_insert(int, list, 3, 4);
    expect = (int[]) {1, 2, 0, 4, 3};
    for (int i = 0; i < list_len; ++i) {
        assert(list[i] == expect[i]);
    }
    assert(list_len == 5 && list_capacity == 8);

    List_free(list);
}

void t_CritBitTree(void) {
}