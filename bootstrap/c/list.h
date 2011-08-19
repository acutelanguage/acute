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
 * File: list.h
 * Description: Defines a linked list.
 ******************************************************************/

#ifndef __ACUTE__LIST_H__
#define __ACUTE__LIST_H__

/* Internal representation of the list nodes are really irrelevant.
   Use the convenience functions for manipulating the structure. */
typedef struct list_node_s list_node_t;

/* The base list type. */
typedef struct list_s
{
	list_node_t* head;
	list_node_t** tailp;
} list_t;

/* Creates a new list, setting the head to the supplied node.
   Node can be NULL. */
extern list_t* list_new(list_node_t*);
/* Free the list */
extern void list_free(list_t*);

/* Add an item to the beginning of list. */
extern void list_prepend(list_t*, void*);
extern void list_prepend_node(list_t*, list_node_t*);

/* Add an item to the end of a list. */
extern void list_append(list_t*, void*);
extern void list_append_node(list_t*, list_node_t*);

/* Remove a node from the list. */
extern void list_remove(list_t*, list_node_t*);

#endif /* !__ACUTE__LIST_H__ */
