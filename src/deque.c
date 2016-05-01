#include <stdlib.h>
#include <assert.h>
#include "deque.h"

typedef struct refnode_s
{
    struct refnode_s* next;
    struct refnode_s* prev;
    refhdr_t*         obj;
} refnode_t;

struct refdeque_s
{
    refnode_t* head;
    refnode_t* tail;
};

refdeque_t* refdeque_alloc()
{
    refdeque_t* deque = malloc(sizeof(refdeque_t));
    if(deque != NULL) {
        deque->head = deque->tail = NULL;
    }
    return deque;
}

void refdeque_release(refdeque_t* deque)
{
    free(deque);
    deque = NULL;
}

bool refdeque_empty(refdeque_t* deque)
{
    return deque->head == NULL;
}

void refdeque_push_front(refdeque_t* deque, refhdr_t* hdr)
{
    refnode_t* node = malloc(sizeof(refnode_t));
    assert(node != NULL);
    node->obj  = hdr;
    node->next = deque->head;
    node->prev = NULL;
    if(deque->tail == NULL) {
        deque->head = deque->tail = node;
    } else {
        deque->head->prev = node;
        deque->head       = node;
    }
}

void refdeque_push_back(refdeque_t* deque, refhdr_t* hdr)
{
    refnode_t* node = malloc(sizeof(refnode_t));
    assert(node != NULL);
    node->obj  = hdr;
    node->prev = deque->tail;
    node->next = NULL;
    if(deque->head == NULL) {
        deque->head = deque->tail = node;
    } else {
        deque->tail->next = node;
        deque->tail       = node;
    }
}

refhdr_t* refdeque_pop_front(refdeque_t* deque)
{
    if(deque->head == NULL) {
        return NULL;
    }

    refhdr_t* hdr   = deque->head->obj;
    refnode_t* node = deque->head;
    if(deque->head == deque->tail) {
        deque->head = deque->tail = NULL;
    } else {
        deque->head = node->next;
    }
    free(node);
    return hdr;
}

refhdr_t* refdeque_pop_back(refdeque_t* deque)
{
    if(deque->tail == NULL) {
        return NULL;
    }

    refhdr_t* hdr   = deque->tail->obj;
    refnode_t* node = deque->tail;
    if(deque->tail == deque->head) {
        deque->tail = deque->head = NULL;
    } else {
        deque->tail = node->prev;
    }
    free(node);
    return hdr;
}

void refdeque_foreach(refdeque_t* deque, void (^each)(refhdr_t* hdr))
{
    if(deque->head == NULL) {
        return;
    }
    
    refnode_t* current = deque->head;
    while(current != NULL) {
        if(each != NULL) {
            each(current->obj);
        }
        current = current->next;
    }
}
