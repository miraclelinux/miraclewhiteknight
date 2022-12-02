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

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/limits.h>
#include <sys/stat.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include <tee_client_api.h>
#include "tee.h"
#include "list.h"
#include "log.h"
#include "ta/user_ta_header_defines.h"

/**
 * Initialize whitelist on tee
 * @param [in] TEEC_Session (*sess)
 * @return status code
*/
int tee_wl_init (TEEC_Session *sess) {
	TEEC_Operation op;
	TEEC_Result res;
	uint32_t rorig;
	op.paramTypes = TEEC_PARAM_TYPES (
							TEEC_NONE,
							TEEC_NONE,
							TEEC_NONE,
							TEEC_NONE);
	res = TEEC_InvokeCommand (sess, TEE_WL_INIT, &op, &rorig);
	if (res != TEEC_SUCCESS) {
		log_err ("%s: invoke failed: %s (code 0x%x origin 0x%x)", __FUNCTION__, tee_strerror (res), res, &rorig);
		return -1;
	}
	return 0;
}

/**
 * Add whitelist to tee.
 * @param (*sess)
 * @param (*path)	path to whitelist file
 * @return
*/
int tee_wl_append (TEEC_Session *sess, const char *path) {
	TEEC_Operation op;
	TEEC_Result res;
	uint32_t rorig;
	int fd;
	struct stat st;
	void *data;


	if ( (fd = open (path, O_RDONLY)) == -1 )
		return -1;

	if ( fstat (fd, &st) != 0 )
		return -1;

	data = malloc (st.st_size + 1);

	if ( read (fd, data, st.st_size) == -1 )
		return -1;

	((char *)data)[st.st_size + 1] = '\0';

	op.paramTypes = TEEC_PARAM_TYPES (
							TEEC_MEMREF_TEMP_INPUT,
							TEEC_NONE,
							TEEC_NONE,
							TEEC_NONE);
	op.params[0].tmpref.buffer = (void *)data;
	op.params[0].tmpref.size = st.st_size;

	res = TEEC_InvokeCommand (sess, TEE_WL_APPEND, &op, &rorig);
	if (res != TEEC_SUCCESS) {
		log_err ("%s: invoke failed: %s (code 0x%x origin 0x%x)", __FUNCTION__, tee_strerror (res), res, &rorig);
		return -1;
	}
	return 0;
}

int tee_wl_remove (TEEC_Session *sess, const char *path) {
	return TEEC_SUCCESS;
}

int tee_wl_flush (void) {
	return TEEC_SUCCESS;
}

int tee_wl_verify (TEEC_Session *sess, const char *path, const unsigned char *hash) {
	TEEC_Operation op;
	TEEC_Result res;
	uint32_t rorig;

	op.paramTypes = TEEC_PARAM_TYPES (
							TEEC_MEMREF_TEMP_INPUT,
							TEEC_MEMREF_TEMP_INPUT,
							TEEC_NONE,
							TEEC_NONE);
	op.params[0].tmpref.buffer = (char *)path;
	op.params[0].tmpref.size = strlen (path) + 1;
	op.params[1].tmpref.buffer = (unsigned char *)hash;
	op.params[1].tmpref.size = 65;

	res = TEEC_InvokeCommand (sess, TEE_WL_VERIFY, &op, &rorig);
	if ( res == TEEC_ERROR_NO_ENTRY ) {
		return -3;
	} else if ( res == TEEC_ERROR_INVALID_HASH ) {
		return -2;
	} else if ( res == TEEC_ACCESS_GRANTED ) {
		return 0;
	} else {
		log_err ("%s: invoke failed: %s (code 0x%x origin 0x%x)", __FUNCTION__, tee_strerror (res), res, &rorig);
		return -1;
	}
}

int tee_ml_add (TEEC_Session *sess, const char *path) {
	int fd;
	struct stat st;
	void *data;
	TEEC_Operation op;
	TEEC_Result res;
	uint32_t rorig;

	if ( (fd = open (path, O_RDONLY)) == -1 )
		return -1;

	if ( fstat (fd, &st) != 0 )
		return -1;

	data = malloc (st.st_size);

	if ( read (fd, data, st.st_size) == -1 )
		return -1;

	op.paramTypes = TEEC_PARAM_TYPES (
							TEEC_MEMREF_TEMP_INPUT,
							TEEC_NONE,
							TEEC_NONE,
							TEEC_NONE);
	op.params[0].tmpref.buffer = data;
	op.params[0].tmpref.size = st.st_size;
	res = TEEC_InvokeCommand (sess, TEE_ML_ADD, &op, &rorig);
	if (res != TEEC_SUCCESS) {
		log_err ("%s: invoke failed: %s (code 0x%x origin 0x%x)", __FUNCTION__, tee_strerror (res), res, &rorig);
		return -1;
	}

	close (fd);
	return 0;
}

// int tee_ml_remove (TEEC_Session *sess, const char *path) {
// 	return TEEC_SUCCESS;
// }

// int tee_ml_flush (TEEC_Session *sess) {
// 	return TEEC_SUCCESS;
// }

/*
 * Get mount point list from tee
*/
int tee_ml_get_list (TEEC_Session *sess, node_t *list) {
	TEEC_Operation op;
	TEEC_Result res;
	uint32_t rorig;
	char *data, *p;
	uint32_t size;
	char path[PATH_MAX];
	char id[] = "marklist";

	errno = 0;

	size = tee_get_objsize (sess, id);
	if ( (data = (char *)malloc (size)) == NULL )
		return -1;

	memset (&op, 0, sizeof(op));

	op.paramTypes = TEEC_PARAM_TYPES (
							TEEC_MEMREF_TEMP_OUTPUT,
							TEEC_NONE,
							TEEC_NONE,
							TEEC_NONE);
	op.params[0].tmpref.buffer = (void *)data;
	op.params[0].tmpref.size = size;

	res = TEEC_InvokeCommand (sess, TEE_ML_GET_LIST, &op, &rorig);
	if (res != TEEC_SUCCESS) {
		log_err ("%s: invoke failed: %s (code 0x%x origin 0x%x)", __FUNCTION__, tee_strerror (res), res, &rorig);
		return -1;
	}

	for ( p = data; sscanf (p, "%4095s", path) != EOF; p += (strlen (path) + 1) ) {
		list_append (list, path, sizeof (path));
	}

	free (data);

	return 0;
}

/**
 * Regist key to tee
 * @return return 0 if success
*/
int tee_key_regist (TEEC_Session *sess, const char *path, unsigned int *keyid) {
	TEEC_Operation op;
	TEEC_Result res;
	uint32_t rorig;
	int fd;
	struct stat st;
	void *buf;

	errno = 0;
	memset (&op, 0, sizeof(op));

	if ( (fd = open (path, O_RDONLY)) == -1 )
		return -1;

	if ( fstat (fd, &st) != 0 )
		return -1;

	buf = malloc (st.st_size);

	if ( read (fd, buf, st.st_size) == -1 )
		return -1;

	op.paramTypes = TEEC_PARAM_TYPES (
							TEEC_MEMREF_TEMP_INPUT,
							TEEC_VALUE_OUTPUT,
							TEEC_NONE,
							TEEC_NONE);
	op.params[0].tmpref.buffer = buf;
	op.params[0].tmpref.size = st.st_size;
	res = TEEC_InvokeCommand (sess, TEE_KEY_REGIST, &op, &rorig);
	if (res != TEEC_SUCCESS) {
		log_err ("%s: invoke failed: %s (code 0x%x origin 0x%x)", __FUNCTION__, tee_strerror (res), res, &rorig);
		return -1;
	}

	*keyid = op.params[1].value.a;

	free (buf);
	return 0;
}

/**
 * Unregist key to tee
 * @param (*sess)
 * @param (keyid)	specify keyid
 * @return return 0 if success
*/
int tee_key_unregist (TEEC_Session *sess, unsigned int keyid) {
	TEEC_Operation op;
	TEEC_Result res;
	uint32_t rorig;

	op.paramTypes = TEEC_PARAM_TYPES (
							TEEC_VALUE_OUTPUT,
							TEEC_NONE,
							TEEC_NONE,
							TEEC_NONE);
	op.params[0].value.a = keyid;
	res = TEEC_InvokeCommand (sess, TEE_KEY_UNREGIST, &op, &rorig);
	if (res != TEEC_SUCCESS) {
		log_err ("%s: invoke failed: %s (code 0x%x origin 0x%x)", __FUNCTION__, tee_strerror (res), res, &rorig);
		return -1;
	}
	return 0;
}

/**
 * list keys
 * @param (*sess)
 * @param (*list)
 * @return return 0 if success
*/
int tee_key_list (TEEC_Session *sess) {
	TEEC_Operation op;
	TEEC_Result res;
	uint32_t rorig;

	op.paramTypes = TEEC_PARAM_TYPES (
							TEEC_NONE,
							TEEC_NONE,
							TEEC_NONE,
							TEEC_NONE);
	res = TEEC_InvokeCommand (sess, TEE_KEY_LIST, &op, &rorig);
	if (res != TEEC_SUCCESS) {
		log_err ("%s: invoke failed: %s (code 0x%x origin 0x%x)", __FUNCTION__, tee_strerror (res), res, &rorig);
		return -1;
	}
	return 0;
}


int tee_change_mgtmode (TEEC_Session *sess, bool flag) {
	TEEC_Operation op;
	TEEC_Result res;
	uint32_t rorig;

	op.paramTypes = TEEC_PARAM_TYPES (
							TEEC_VALUE_INPUT,
							TEEC_NONE,
							TEEC_NONE,
							TEEC_NONE);
	op.params[0].value.a = flag;
	res = TEEC_InvokeCommand (sess, TEE_CH_MGTMODE, &op, &rorig);
	if (res != TEEC_SUCCESS) {
		log_err ("%s: invoke failed: %s (code 0x%x origin 0x%x)", __FUNCTION__, tee_strerror (res), res, &rorig);
		return -1;
	}
	return 0;
}

/**
 * Get object size on secure storage
 * @param (sess)	session
 * @param (id)		id
 * @return			return size if success. Otherwise, -1 shall be returned and errno set to indicate the error.
*/
int tee_get_objsize (TEEC_Session *sess, const char *id) {
	TEEC_Operation op;
	TEEC_Result res;
	uint32_t rorig;

	op.paramTypes = TEEC_PARAM_TYPES (
							TEEC_MEMREF_TEMP_INPUT,
							TEEC_VALUE_OUTPUT,
							TEEC_NONE,
							TEEC_NONE);
	op.params[0].tmpref.buffer = (void *)id;
	op.params[0].tmpref.size = strlen (id);
	res = TEEC_InvokeCommand (sess, TEE_GET_OBJSIZE, &op, &rorig);
	if (res != TEEC_SUCCESS) {
		log_err ("%s: invoke failed: %s (code 0x%x origin 0x%x)", __FUNCTION__, tee_strerror (res), res, &rorig);
		return -1;
	}

	return op.params[1].value.a;
}

/**
 * Generate a key pair of RSA on TEE
 * @param [in] sess session
 * @param [in] id object id
 * @return			return size if success. Otherwise, -1 shall be returned and errno set to indicate the error.
*/
int tee_key_gen (TEEC_Session *sess) {
	TEEC_Operation op;
	TEEC_Result res;
	uint32_t rorig;

	op.paramTypes = TEEC_PARAM_TYPES (
							TEEC_NONE,
							TEEC_NONE,
							TEEC_NONE,
							TEEC_NONE);
	res = TEEC_InvokeCommand (sess, TEE_KEY_GEN, &op, &rorig);
	if (res != TEEC_SUCCESS) {
		log_err ("%s: invoke failed: %s (code 0x%x origin 0x%x)", __FUNCTION__, tee_strerror (res), res, &rorig);
		return -1;
	}

	// int tmp = tee_key_get_pub (sess);
	// if (tmp != 0) {
	// 	log_err ("%s: %s", __FUNCTION__, strerror (errno));
	// }
	return 0;
}

/**
 * Get object size on secure storage
 * @param (sess)	session
 * @param (id)		id
 * @return			return size if success. Otherwise, -1 shall be returned and errno set to indicate the error.
*/
int tee_key_get_pub (TEEC_Session *sess) {
	TEEC_Operation op;
	TEEC_Result res;
	uint32_t rorig;
	void *data;
	uint32_t data_size;

	if ( (data_size = tee_get_objsize (sess, "keypair")) == -1 )
		return -1;

	if ( (data = malloc (data_size)) == NULL )
		return -1;

	op.paramTypes = TEEC_PARAM_TYPES (
							TEEC_MEMREF_TEMP_OUTPUT,
							TEEC_NONE,
							TEEC_NONE,
							TEEC_NONE);
	op.params[0].tmpref.buffer = data;
	op.params[0].tmpref.size = data_size;
	res = TEEC_InvokeCommand (sess, TEE_KEY_GET_PUB, &op, &rorig);
	if (res != TEEC_SUCCESS) {
		log_err ("%s: invoke failed: %s (code 0x%x origin 0x%x)", __FUNCTION__, tee_strerror (res), res, &rorig);
		return -1;
	}

	log_debug ("key: %s", (char *)data);
	log_debug ("key size: %d", data_size);

	return 0;
}


	// op.params[0].tmpref.buffer = data;
	// op.params[0].tmpref.size = data_size;
	// log_debug ("%s", (char *)data);


char *tee_strerror (TEEC_Result res) {
	switch (res) {
		case TEEC_ERROR_ITEM_NOT_FOUND:
			return "Specified storage id does not exist";
		case TEEC_ERROR_BAD_PARAMETERS:
			return "Bad parameters";
		case TEEC_ERROR_NOT_SUPPORTED:
			return "Not supported";
		case TEEC_ERROR_ACCESS_DENIED:
			return "Access denied";
		case TEEC_ERROR_SHORT_BUFFER:
			return "Buffer is short";
		case TEEC_ERROR_ACCESS_CONFLICT:
			return "Access conflicted";
		case TEEC_ERROR_TARGET_DEAD:
			return "TA is dead";
		default:
			return NULL;
	}
}
