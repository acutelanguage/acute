#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "refuse.h"

static inline refhdr_t* _refuse_header(void* ptr)
{
    return (refhdr_t*)((char*)ptr - sizeof(refhdr_t));
}

void refuse_init(refuse_t* refuse)
{
    refuse->dec = refdeque_alloc();
    refuse->mod = refdeque_alloc();
}

void refuse_destroy(refuse_t* refuse)
{
    refdeque_release(refuse->dec);
    refuse->dec = NULL;
    refdeque_release(refuse->mod);
    refuse->mod = NULL;
}

void refuse_reconcile(refuse_t* refuse)
{
    refhdr_t* h = refdeque_pop_front(refuse->mod);
    while(h != NULL) {
        if(h->new) {
            h->new = 0;
        }

        if(h->dirty) {
            h->dirty = 0;
        } else {
            h->retainCount++;
        }
        h = refdeque_pop_front(refuse->mod);
    }

    refhdr_t* hdr = refdeque_pop_front(refuse->dec);
    while(hdr != NULL) {
        hdr->retainCount--;
        if(hdr->retainCount == 0) {
            // Iterate children, delete children
            free(hdr);
            hdr = NULL;
        }
        hdr = refdeque_pop_front(refuse->dec);
    }
}

void* refuse_alloc(__unused refuse_t* refuse, size_t size)
{
    refhdr_t* o = (refhdr_t*)calloc(sizeof(refhdr_t) + size, 1);
    char* ptr   = (char*)o + sizeof(refhdr_t);

    o->new           = 1;
    o->dirty         = 0;
    o->retainCount   = 0;

    return ptr;
}

void refuse_set_dirty(__unused refuse_t* refuse, void* ptr)
{
    refhdr_t* hdr = _refuse_header(ptr);
    hdr->dirty    = 1;
}

void refuse_retain(refuse_t* refuse, void* ptr)
{
    refhdr_t* o;
    char*     cptr  = (char*)ptr;
    cptr           -= sizeof(refhdr_t);
    o               = (refhdr_t*)cptr;
    
    if(o->dirty) {
        return;
    }

    refdeque_push_back(refuse->mod, o);
}

void refuse_release(refuse_t* refuse, void* ptr)
{
    refhdr_t* hdr = _refuse_header(ptr);
    refdeque_push_back(refuse->dec, hdr);
}