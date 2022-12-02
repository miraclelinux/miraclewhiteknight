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
#ifndef _LIST_
#define _LIST_

#include <stddef.h>

struct node {
	void *data;
	size_t size;
	struct node *next;
};

typedef struct node node_t;
typedef int (* cmp_f) (void *);

node_t *list_init (void);
int list_append (node_t *, void*, size_t size);
int list_show (node_t *);
int list_is_head (node_t *);
int list_is_tail (node_t *);
node_t *list_lastnode (node_t *);
node_t *list_find (node_t *, cmp_f);
int list_remove (node_t *, node_t *);
int list_free (node_t *);
int list_is_empty (node_t *);

#endif
