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
#pragma once

#include "config.h"
#ifndef CONFIG_TA
#include <linux/limits.h>
#include <stdbool.h>
#include "list.h"
#ifdef CONFIG_TEE
#include <tee_client_api.h>
#endif

/* common variables */
extern node_t *whitelist;
extern bool debug;
#ifdef CONFIG_TEE
TEEC_Context ctx;
TEEC_Session sess;
#endif

/* structures */
struct wl_entry {
	char filepath[PATH_MAX];
	/* TODO: define hash length */
	unsigned char hash[65];
};

typedef struct wl_entry wl_entry_t;

#define LIST_DIR "wl.d"
#define TARGETS_DIR "ml.d"
#endif

/* chomp : removes the last carriage return of string. */
/* chomp(char *string) */
void chomp(char *);
int strsplit (const char *, const char *, char *, unsigned int, char *, unsigned int);
