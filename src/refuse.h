// Refuse
// Copyright © 2016, Jeremy Tregunna, All Rights Reserved.

#ifndef __REFUSE__REFUSE_H__
#define __REFUSE__REFUSE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "types.h"
#include "deque.h"

typedef struct
{
    refdeque_t* dec;
    refdeque_t* mod;
} refuse_t;

// Initialize the garbage collector. It should be noted, that refuse is
// not yet thread safe. Therefore it's recommended that refuse run in
// its own thread. Objects may be shared across thread boundaries for
// the purposes of your use case, however, all operations, save for
// refuse_alloc, must be run on the thread that owns refuse.
extern void refuse_init(refuse_t*);

// Allocate a chunk of memory and place it under control of an instance
// of refuse. The pointer returned points at the start of memory for
// your object, and you do not need to supply any header yourself.
extern void* refuse_alloc(refuse_t*, size_t);

// Destroy the garbage collector. This will run the reconciler, and
// deallocate all internal state. It is not safe to use your refuse
// instance after calling this function.
extern void refuse_destroy(refuse_t*);

// Mark an object as dirty. This has the benefit of avoiding a retain
// call for each modification to the parameter you do, until the
// reconciler runs. At that point, the dirty bit will be reset.
extern void refuse_set_dirty(refuse_t*, void*);

// Reconcile the retains and releases. This must be done at periodic
// intervals, i.e., when returning from a function, or somesuch. It
// should be done often enough that you never have too many items
// queued up, but not too often as to negate the risk of the
// optimizations this library provides on top of reference counting.
//
// Measure twice.
extern void refuse_reconcile(refuse_t*);

// Mark an object for retain. It will not be retained immediately,
// but you are guaranteed to retain the object before any release will
// be applied against it.
extern void refuse_retain(refuse_t*, void*);

// Mark an object for release. This object will not be released
// immediately, but at some indeterminate point in the future.
extern void refuse_release(refuse_t*, void*);

#ifdef __cplusplus
}
#endif

#endif // !__REFUSE__REFUSE_H__
