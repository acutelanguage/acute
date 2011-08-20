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
	list_node_t* prev;
	void* data;
};

list_node_t* list_node_new(void* value)
{
	list_node_t* r = malloc(sizeof(*r));
	r->data = value;
	r->next = NULL;
	r->prev = NULL;
	return r;
}

void list_node_free(list_node_t* node)
{
	free(node);
	node = NULL;
}

list_t* list_new(list_node_t* node)
{
	list_t* list = malloc(sizeof(*list));

	if(list)
	{
		list->head = node;
		list->tailp = &list->head;
	}

	return list;
}

void list_free(list_t* list)
{
	list_node_t* node = NULL;
	for(node = list->head; node != NULL;)
	{
		list_node_t* tmp = list->head;
		list->head = node->next;
		free(tmp);
	}
	free(list);
	list = NULL;
}

void list_prepend_node(list_t* list, list_node_t* node)
{
	list_node_t* head = list->head;
	node->prev = NULL;
	node->next = head;
	head->prev = node;
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
	node->prev = *list->tailp;
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

void* list_pop(list_t* list)
{
	list_node_t* last = *list->tailp;
	*list->tailp = last->prev;
	list->tailp = &last->prev;
	return last;
}
