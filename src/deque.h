// Refuse
// Copyright © 2016, Jeremy Tregunna, All Rights Reserved.

#ifndef __REFUSE__DEQUE_H__
#define __REFUSE__DEQUE_H__

#include <stdbool.h>
#include "types.h"

typedef struct refdeque_s refdeque_t;

refdeque_t* refdeque_alloc();
void refdeque_release(refdeque_t*);
bool refdeque_empty(refdeque_t*);
void refdeque_push_front(refdeque_t*, refhdr_t*);
void refdeque_push_back(refdeque_t*, refhdr_t*);
refhdr_t* refdeque_pop_front(refdeque_t*);
refhdr_t* refdeque_pop_back(refdeque_t*);
void refdeque_foreach(refdeque_t*, void (^)(refhdr_t*));

#endif // !__REFUSE__DEQUE_H__