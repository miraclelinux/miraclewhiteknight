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

#include <tee_client_api.h>
#include <stdbool.h>
#include "list.h"

uint32_t rorig;
TEEC_Result teeres;

int tee_wl_init (TEEC_Session *);
int tee_wl_append (TEEC_Session *, const char *);
int tee_wl_remove (TEEC_Session *, const char *);
int tee_wl_flush (void);
int tee_wl_verify (TEEC_Session *, const char *, const unsigned char *);
int tee_ml_add (TEEC_Session *, const char *);
int tee_ml_remove (TEEC_Session *, const char *);
int tee_ml_flush (TEEC_Session *);
int tee_ml_get_list (TEEC_Session *, node_t *);
int tee_key_regist (TEEC_Session *, const char *, unsigned int *);
int tee_change_mgtmode (TEEC_Session *, bool);
int tee_get_objsize (TEEC_Session *, const char *);
int tee_key_unregist (TEEC_Session *, unsigned int);
int tee_key_list (TEEC_Session *);
int tee_key_gen (TEEC_Session *);
int tee_key_get_pub (TEEC_Session *);
char *tee_strerror (TEEC_Result);
