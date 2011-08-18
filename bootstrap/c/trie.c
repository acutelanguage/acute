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
 * File: trie.c
 * Description: Implementation of a PATRICIA trie
 ******************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "trie.h"

static inline int bit_get(const char* key, int n)
{
	int k;
	if(n < 0)
		return 2; // psuedo-bit
	k = (n & 0x7);
	return ((*(key + (n >> 3))) >> k) & 0x1;
}

static inline int key_compare(const char* k1, const char* k2)
{
	if(!k1 || !k2)
		return 0;
	return (strcmp(k1, k2) == 0);
}

static inline int bit_first_different(const char* k1, const char* k2)
{
	int n, d;
	if(!k1 || !k2)
		return 0; // First bit is different
	n = d = 0;
	while((k1[n] == k2[n]) && (k1[n] != 0) && (k2[n] != 0))
		n++;
	while(bit_get(&k1[n], d) == bit_get(&k2[n], d))
		d++;
	return ((n << 3) + d);
}

static inline void key_copy(trie_node_t* src, trie_node_t* dst)
{
	if(src == dst)
		return;

	if(strlen(dst->key) < strlen(src->key))
		dst->key = realloc(dst->key, strlen(src->key) + 1);
	strcpy(dst->key, src->key);

	dst->data = src->data;
}

static inline void recursive_remove(trie_t* trie, trie_node_t* node)
{
	trie_node_t* l = node->left;
	trie_node_t* r = node->right;

	// Remove left branch
	if((l->bit_index >= node->bit_index) && (l != node) && (l != trie->head))
		recursive_remove(trie, l);

	// Remove right branch
	if((r->bit_index >= node->bit_index) && (r != node) && (r != trie->head))
		recursive_remove(trie, r);

	free(node);
	node = NULL;
}

trie_node_t* trie_node_new(const char* key, void* data, int bit_index, trie_node_t* left, trie_node_t* right)
{
	trie_node_t* node = malloc(sizeof(*node));

	if(node)
	{
		if(key)
			node->key = strdup(key);
		else
			node->key = (char*)key;
		node->data = data;
		node->bit_index = bit_index;
		node->left = left;
		node->right = right;
	}

	return node;
}

void trie_node_free(trie_node_t* node)
{
	if(node)
	{
		free(node->key);
		node->key = NULL;
	}
}

void trie_node_set_data(trie_node_t* node, void* data, size_t dsize)
{
	memcpy(node->data, data, dsize);
}

/* Trie itself */

trie_t* trie_new(void)
{
	trie_t* trie = malloc(sizeof(*trie));

	if(trie)
	{
		trie->head = trie_node_new(NULL, NULL, -1, NULL, NULL);
		if(trie->head)
			trie->head->key = calloc(256, 1);
	}

	return trie;
}

void trie_free(trie_t* trie)
{
	recursive_remove(trie, trie->head);
	free(trie);
	trie = NULL;
}

trie_node_t* trie_insert(trie_t* trie, const char* key, void* data)
{
	trie_node_t* p;
	trie_node_t* t;
	trie_node_t* x;
	int i;

	if(trie == NULL)
		return NULL;

	p = trie->head;
	t = p->right;

	// Navigate down the trie and look for the key
	while(p->bit_index < t->bit_index)
	{
		p = t;
		t = bit_get(key, t->bit_index) ? t->right : t->left;
	}

	// Is the key already in the trie?
	if(key_compare(key, t->key))
		return NULL; // Already in the trie!

	// Find the first bit that does not match.
	i = bit_first_different(key, t->key);

	// Find the appropriate place in the trie where the node has
	// to be inserted.
	p = trie->head;
	x = p->right;
	while((p->bit_index < x->bit_index) && (x->bit_index < i))
	{
		p = x;
		x = bit_get(key, x->bit_index) ? x->right : x->left;
	}

	// Allocate a new node and initialize it.
	t = trie_node_new(key, data, i, (bit_get(key, i) ? x : t), (bit_get(key, i) ? t : x));

	// Rewire
	if(bit_get(key, p->bit_index))
		p->right = t;
	else
		p->left = t;

	return t;
}

void* trie_lookup(trie_t* trie, const char* key)
{
	trie_node_t* node = trie_lookup_node(trie, key);
	if(!node)
		return NULL;
	return node->data;
}

trie_node_t* trie_lookup_node(trie_t* trie, const char* key)
{
	trie_node_t* p;
	trie_node_t* x;

	p = trie->head;
	x = trie->head->right;

	// Go down the trie structure until an upward link is encountered
	while(p->bit_index < x->bit_index)
	{
		p = x;
		x = bit_get(key, x->bit_index) ? x->right : x->left;
	}

	// Perform a full string comparison, and return NULL if they key is
	// not found at this location in the structure.
	if(!key_compare(key, x->key))
		return NULL;

	return x;
}

int trie_delete(trie_t* trie, const char* key)
{
	trie_node_t* p;
	trie_node_t* t;
	trie_node_t* x;
	trie_node_t* pp;
	trie_node_t* lp;
	int bp, bl, br;
	char* k = NULL;

	p = trie->head;
	t = p->right;

	// Look for the key
	while(p->bit_index < t->bit_index)
	{
		pp = p;
		p = t;
		t = bit_get(key, t->bit_index) ? t->right : t->left;
	}

	// Is this the key in the trie? if not, get out
	if(!key_compare(key, t->key))
		return 0;

	// Copy p's key to t
	if(t != p)
		key_copy(p, t);

	// Is p a leaf?
	bp = p->bit_index;
	bl = p->left->bit_index;
	br = p->right->bit_index;
	if((bl > bp) || (br > bp))
	{
		// At least one downward edge
		if(p != t)
		{
			// Look for a new (intermediate) key
			k = strdup(p->key);

			lp = p;
			x = bit_get(k, p->bit_index) ? p->right : p->left;
			while(lp->bit_index < x->bit_index)
			{
				lp = x;
				x = bit_get(k, x->bit_index) ? x->right : x->left;
			}

			// If the intermediate key wasn't found, we have a problem.
			if(!key_compare(k, x->key))
			{
				free(k);
				return 0;
			}

			// Rewire the leaf (lp) to point to t
			if(bit_get(k, lp->bit_index))
				lp->right = t;
			else
				lp->left = t;
		}

		// Rewire the parent to point to the real child of p
		if(pp != p)
		{
			trie_node_t* ch = bit_get(key, p->bit_index) ? p->left : p->right;
			if(bit_get(key, pp->bit_index))
				pp->right = ch;
			else
				pp->left = ch;
		}

		// Don't need 'k' anymore
		free(k);
		k = NULL;
	}
	else
	{
		// Both edges (left, right) are pointing upwards or to the node (self-edges).

		// Rewire the parent.
		if(pp != p)
		{
			trie_node_t* blx = p->left;
			trie_node_t* brx = p->right;
			if(bit_get(key, pp->bit_index))
				pp->right = (((blx == brx) && (blx == p)) ? pp : ((blx == p) ? brx : blx));
			else
				pp->left = (((blx == brx) && (blx == p)) ? pp : ((blx == p) ? brx : blx));
		}
	}

	// Don't need p anymore
	free(p);
	p = NULL;

	return 1;
}
