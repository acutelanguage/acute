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

void gc_collect(gc_t* gc)
{
	gc_mark_heap(gc);
	gc_sweep(gc);
}

void _gc_mark_stack_foreach(void* r, void* ctx)
{
	obj_t* root = (obj_t*)r;
	gc_t* gc = (gc_t*)ctx;
	for(obj_t* m = (obj_t*)judy_strt(root->slots, (unsigned char*)"", 0); m != NULL; m = (obj_t*)judy_nxt(root->slots))
	{
		if(m->marked == 0)
			m->marked = 1;
		list_push(gc->mark_stack, m);
	}
}

void _gc_roots_foreach(void* r, void* ctx)
{
	obj_t* root = (obj_t*)r;
	gc_t* gc = (gc_t*)ctx;
	root->marked = 1;
	list_push(gc->mark_stack, root);
	list_foreach(gc->mark_stack, kListIterateDirectionReverse, gc, _gc_mark_stack_foreach);
}

void gc_mark_heap(gc_t* gc)
{
	list_node_t* head = list_node_new(NULL); // Sentinel, because while my list is efficient, it doesn't like to be empty.
	if(gc->mark_stack)
		list_free(gc->mark_stack);
	gc->mark_stack = list_new(head);

	list_foreach(gc->roots, kListIterateDirectionReverse, gc, _gc_roots_foreach);
}

void _gc_sweep_foreach(void* o, void* ctx)
{
	obj_t* obj = (obj_t*)o;
	gc_t* gc = (gc_t*)ctx;

	if(obj->marked)
		obj->marked = 0;
	else
		obj_release(obj); // Could use a free list here, not going to though.
}

void gc_sweep(gc_t* gc)
{
	list_foreach(gc->mark_stack, kListIterateDirectionReverse, gc, _gc_sweep_foreach);
}
