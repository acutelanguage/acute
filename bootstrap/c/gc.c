/*******************************************************************
 * Acute Programming Language
 * Copyright (C) 2011, Jeremy Tregunna, All Rights Reserved.
 *
 * This software project, which includes this module, is protected
 * under Canadian copyright legislation, as well as international
 * treaty. It may be used only as directed by the copyright holder.
 * The copyright holder hereby consents to usage and distribution
 * based on the terms and conditions of the MIT license, which may
 * be found in the LICENSE.MIT file included in this distribution.
 *******************************************************************
 * Project: Acute Programming Language
 * File: gc.h
 * Description: Simple mark-sweep garbage collector. No generations,
 *              No threading support, stop the world. So 21st
 *              century it's not funny!
 ******************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "gc.h"
#include "object.h"
#include "list.h"
#include "judy64d.h"

struct gc_s
{
	size_t  max_heap_size;
	size_t  current_heap_size;
	list_t* mark_stack;
	list_t* roots;
	list_t* free_list;
};

static gc_t* __acute_collector = NULL;

static void _gc_mark_stack_foreach(void* r, void* ctx)
{
	gcable_t* root = (gcable_t*)r;
	gcable_t* m = NULL;
	for(m = (gcable_t*)judy_strt(root->slots, (unsigned char*)"", 0); m != NULL; m = (gcable_t*)judy_nxt(root->slots))
	{
		if(m->marked == 0)
			m->marked = 1;
		list_push(__acute_collector->mark_stack, m);
	}
}

static void _gc_roots_foreach(void* r, void* ctx)
{
	gcable_t* root = (gcable_t*)r;
	root->marked = 1;
	list_push(__acute_collector->mark_stack, root);
	list_foreach(__acute_collector->mark_stack, kListIterateDirectionReverse, NULL, _gc_mark_stack_foreach);
}

static void gc_mark_heap(gc_t* gc)
{
	list_node_t* head = list_node_new(NULL); // Sentinel, because while my list is efficient, it doesn't like to be empty.
	if(gc->mark_stack)
		list_free(gc->mark_stack);
	gc->mark_stack = list_new(head);

	list_foreach(gc->roots, kListIterateDirectionReverse, gc, _gc_roots_foreach);
}

static void _gc_sweep_foreach(void* o, void* ctx)
{
	gcable_t* obj = (gcable_t*)o;
	gc_t* gc = (gc_t*)ctx;

	if(obj->marked)
		obj->marked = 0;
	else
		list_append(gc->free_list, obj);
}

static void gc_sweep(gc_t* gc)
{
	list_foreach(gc->mark_stack, kListIterateDirectionReverse, gc, _gc_sweep_foreach);
}

void gc_initialize(size_t max_heap_size)
{
	if(__acute_collector == NULL)
		__acute_collector = malloc(sizeof(__acute_collector));
	__acute_collector->max_heap_size = max_heap_size;
	__acute_collector->current_heap_size = 0;
}

void gc_destroy(void)
{
	list_free(__acute_collector->mark_stack);
	list_free(__acute_collector->roots);
	free(__acute_collector);
	__acute_collector = NULL;
}

void* gc_alloc(size_t size)
{
	struct gcable_s* ptr = NULL;

	if(size >= (2 ^ 28)) // Hard limit of object sizes: 256 MB
		return NULL;

	if(__acute_collector->current_heap_size + size >= __acute_collector->max_heap_size)
		gc_collect();

	ptr = list_pop(__acute_collector->free_list);
	if(!ptr)
		ptr = malloc(sizeof(size));
	if(ptr)
	{
		__acute_collector->current_heap_size += size;
		ptr->object_size = size;
		ptr->marked = 0;
	}

	return (void*)ptr;
}

void gc_collect(void)
{
	gc_mark_heap(__acute_collector);
	gc_sweep(__acute_collector);
}

void gc_add_root(void* root)
{
	list_append(__acute_collector->roots, root);
}
