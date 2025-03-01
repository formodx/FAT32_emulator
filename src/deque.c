#include "deque.h"


void create_deque(struct deque **dq, int cnt){
    *dq = malloc(sizeof(struct deque));
    (*dq)->count = cnt;
    (*dq)->data = malloc(sizeof(uint64_t) * cnt);
}


void delete_deque(struct deque **dq){
    assert((*dq) != NULL);

    if(*dq == NULL) return;

    (*dq)->count = 0;
    free((*dq)->data);
    free(*dq);
    *dq = NULL;
}


void push_front_deque(struct deque **dq, uint64_t x){
    struct deque *temp = make_copy_deque(dq);
    delete_deque(dq);
    create_deque(dq, temp->count+1);
    for(int i=0; i<temp->count; ++i) (*dq)->data[i+1] = temp->data[i];
    (*dq)->data[0] = x;

    delete_deque(&temp);
}


void push_back_deque(struct deque **dq, uint64_t x){
    struct deque *temp = make_copy_deque(dq);
    delete_deque(dq);
    create_deque(dq, temp->count+1);
    for(int i=0; i<temp->count; ++i) (*dq)->data[i] = temp->data[i];
    (*dq)->data[temp->count] = x;

    delete_deque(&temp);
}


void pop_front_deque(struct deque **dq){
    assert((*dq)->count);

    struct deque *temp = make_copy_deque(dq);
    delete_deque(dq);
    create_deque(dq, temp->count-1);
    for(int i=1; i<temp->count; ++i) (*dq)->data[i-1] = temp->data[i];

    delete_deque(&temp);
}


void pop_back_deque(struct deque **dq){
    assert((*dq)->count);

    struct deque *temp = make_copy_deque(dq);
    delete_deque(dq);
    create_deque(dq, temp->count-1);
    for(int i=0; i<temp->count-1; ++i) (*dq)->data[i] = temp->data[i];

    delete_deque(&temp);
}


uint64_t get_front_deque(struct deque **dq){
    assert(*dq != NULL);
    assert((*dq)->count);

    return (*dq)->data[0];
}


uint64_t get_back_deque(struct deque **dq){
    assert(*dq != NULL);
    assert((*dq)->count);

    return (*dq)->data[(*dq)->count - 1];
}


struct deque *make_copy_deque(struct deque **dq){
    assert(*dq != NULL);

    struct deque *result = NULL;
    create_deque(&result, (*dq)->count);
    for(size_t i=0; i<(*dq)->count; ++i) result->data[i] = (*dq)->data[i];

    return result;
}