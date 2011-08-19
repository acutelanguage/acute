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

#include <stdlib.h>
#include "list.h"

struct list_node_s
{
	list_node_t* next;
	void* data;
};

list_t* list_new(list_node_t* node)
{
	list_t* list = malloc(sizeof(*list));

	if(list)
	{
		list->head = node;
		list->tailp = &list->head;
		list->queue = q;
	}

	return list;
}

void list_prepend_node(list_t* list, list_node_t* node)
{
	node->next = list->head;
	list->head = node;
	if(list->tailp == &list->head)
		list->tailp = &node->next;
}

void list_prepend(list_t* list, void* item)
{
	list_node_t* node = malloc(sizeof(*node));
	node->data = item;
	list_prepend_node(list, node);
}

void list_append_node(list_t* list, list_node_t* node)
{
	*list->tailp = node;
	list->tailp = &node->next;
}

void list_append(list_t* list, void* item)
{
	list_node_t* node = malloc(sizeof(*node));
	node->data = item;
	node->next = list->head;
	list_append_node(list, node);
}

void list_remove(list_t* list, list_node_t* node)
{
	list_node_t* current;
	list_node_t** pnp = &list->head;

	while((current = *pnp) != NULL)
	{
		if(current == node)
		{
			*pnp = node->next;
			if(list->tailp == &node->next)
				list->tailp = pnp;
			node->next = NULL;
			break;
		}
		pnp = &current->next;
	}
}
