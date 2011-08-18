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
 * File: trie.h
 * Description: Definitions for a PATRICIA trie.
 ******************************************************************/

#ifndef __ACUTE__RUNTIME__TRIE_H__
#define __ACUTE__RUNTIME__TRIE_H__

#include <sys/types.h>

/*
 * This module defines a PATRICIA trie, which we use for storing
 * our slot table on actors.
 */
 
/* Node structure */
typedef struct trie_node_s
{
	int bit_index;
	char* key;
	void* data;
	struct trie_node_s* left;
	struct trie_node_s* right;
} trie_node_t;

/* Trie itself */

typedef struct trie_s
{
	trie_node_t* head;
} trie_t;

#define trie_node_get_left(node) ((node)->left)
#define trie_node_get_right(node) ((node)->right)
#define trie_node_get_key(node) ((node)->key)
#define trie_node_get_data(node) ((node)->data)

extern trie_node_t* trie_node_new(const char*, void*, int, trie_node_t*, trie_node_t*);
extern void trie_node_free(trie_node_t*);
extern void trie_node_set_data(trie_node_t*, void*, size_t);

extern trie_t* trie_new(void);
extern void trie_free(trie_t*);
extern trie_node_t* trie_insert(trie_t*, const char*, void*);
extern void* trie_lookup(trie_t*, const char*);
extern trie_node_t* trie_lookup_node(trie_t*, const char*);
extern int trie_delete(trie_t*, const char*);

#endif /* !__ACUTE__RUNTIME__TRIE_H__ */
