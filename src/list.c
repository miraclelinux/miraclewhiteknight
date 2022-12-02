/*
 * Copyright (C) 2018,2019 Cybertrust Japan Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */
#include <string.h>
#include <stdlib.h>
#include "list.h"

node_t *list_init (void) {
	node_t *head, *tail;
	head = (node_t *) malloc (sizeof (struct node));
	tail = (node_t *) malloc (sizeof (struct node));
	head->next = tail;
	tail->next = tail;
	tail->data = head->data = NULL;

	return head;
}

int list_is_head (node_t *n) {
	return n->data == NULL && n->next != NULL;
}

int list_is_tail (node_t *n) {
	return n->data == NULL && n->next == n;
}

/* find the last node ahead of the tail. */
node_t *list_lastnode (node_t *list) {
	node_t *n = list;
	while ( !list_is_tail (n->next) )
		n = n->next;
	return n;
}

int list_append (node_t *list, void *data, size_t size) {
	node_t *p = list_lastnode (list);
	node_t *new = (node_t *) malloc (sizeof (node_t));
	new->data = (void *) malloc (size);
	new->size = size;
	memcpy (new->data, data, size);
	new->next = p->next;
	p->next = new;
	return 0;
}

node_t *list_find (node_t *list, cmp_f func) {
	node_t *p = list->next;
	while ( !list_is_tail (p) ) {
		if ( func (p->data) ) return p;
		p = p->next;
	}
	return NULL;
}

/*
 * Remove a node from list
 * @param (*list)	list pointer
 * @param (*t)		a node pointer
 * @return			return 0 if found the node and removed it. Otherwise, return -1.
*/
int list_remove (node_t *list, node_t *t) {
	node_t *p = list;
	while ( !list_is_tail (p) ) {
		if (p->next == t) {
			p->next = t->next;
			free (t->data);
			free (t);
			return 0;
		}
		p = p->next;
	}
	return -1;
}

int list_show (struct node *list) { return 0; }

/*
 * Clean list
 * @param(*list)		list pointer
 * @return				always return 0
*/
int list_free (struct node *list) {
	node_t *tmp;

	/* free head and normal nodes */
	while ( ! list_is_tail (list) ) {
		tmp = list;
		list = list->next;
		if ( tmp->data != NULL )
			free (tmp->data);
		free (tmp);
	}

	/* tail node */
	free (list);
	return 0;
}

/*
 * Check list is empty
 * @param (list)
 * return: return 1 if the list is empty. Otherwise, return 0.
*/
int list_is_empty (node_t *list) {
	return list_is_tail (list->next);
}
