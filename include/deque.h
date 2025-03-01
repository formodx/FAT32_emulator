#ifndef LIST_H
#define LIST_H


#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>


struct deque{
    size_t count;
    uint64_t *data;
};


void create_deque(struct deque **dq, int cnt);
void delete_deque(struct deque **dq);
void push_front_deque(struct deque **dq, uint64_t x);
void push_back_deque(struct deque **dq, uint64_t x);
void pop_front_deque(struct deque **dq);
void pop_back_deque(struct deque **dq);
uint64_t get_front_deque(struct deque **dq);
uint64_t get_back_deque(struct deque **dq);
struct deque *make_copy_deque(struct deque **dq);


#endif