#include <tee_internal_api.h>
#include <tee_internal_api_extensions.h>
#include "user_ta_header_defines.h"
#include <string.h>
#include "../common.h"
#include "object.h"
#include "key.h"

/* store whitelist */
void *wl_data;
uint32_t wl_size;
bool mgtmode;

TEE_Result TA_CreateEntryPoint (void) {
	mgtmode = false;
	return TEE_SUCCESS;
}

void TA_DestroyEntryPoint (void) {}

TEE_Result TA_OpenSessionEntryPoint (uint32_t param_types, TEE_Param __unused param[4], void __unused **sess) {
	uint32_t exp_param_types;

	exp_param_types = TEE_PARAM_TYPES (
								TEE_PARAM_TYPE_NONE,
								TEE_PARAM_TYPE_NONE,
								TEE_PARAM_TYPE_NONE,
								TEE_PARAM_TYPE_NONE);

	if (param_types != exp_param_types) return TEE_ERROR_BAD_PARAMETERS;

	DMSG ("session opened");

	return TEE_SUCCESS;
}

void TA_CloseSessionEntryPoint (void __unused *sess) {
	TEE_Free (wl_data);
	DMSG("Goodbye!\n");
}

static TEE_Result tee_wl_init (uint32_t param_types, TEE_Param __unused params[4]) {
	uint32_t exp_param_types;
	char id[] = "whitelist";
	TEE_ObjectInfo objinfo;
	TEE_Result res;

	exp_param_types = TEE_PARAM_TYPES (
								TEE_PARAM_TYPE_NONE,
								TEE_PARAM_TYPE_NONE,
								TEE_PARAM_TYPE_NONE,
								TEE_PARAM_TYPE_NONE);

	if (param_types != exp_param_types) return TEE_ERROR_BAD_PARAMETERS;

	/* read whitelist from storage */
	if ( (res = get_objinfo (id, &objinfo)) != TEE_SUCCESS ) {
		EMSG ("get_objinfo failed, res=0x%08x", res);
		return res;
	}

	DMSG ("expect data size: %d", objinfo.dataSize);
	wl_size = sizeof (char) * objinfo.dataSize;
	wl_data = (char *)TEE_Malloc (wl_size, 0);

	if ( (res = read_data (id, wl_data, &wl_size)) != TEE_SUCCESS ) {
		EMSG ("Failed to read object, res=0x%08x", res);
		return res;
	}

	DMSG ("actual data size: %d", wl_size);

	return TEE_SUCCESS;
}

static TEE_Result tee_wl_append (uint32_t param_types, TEE_Param params[4]) {
	TEE_Result res;
	TEE_ObjectInfo objinfo;
	uint32_t exp_param_types;
	void *key_data = NULL, *decript_data = NULL;
	uint32_t key_size, decript_size;

	exp_param_types = TEE_PARAM_TYPES (
								TEE_PARAM_TYPE_MEMREF_INPUT,
								TEE_PARAM_TYPE_NONE,
								TEE_PARAM_TYPE_NONE,
								TEE_PARAM_TYPE_NONE);

	if (param_types != exp_param_types) return TEE_ERROR_BAD_PARAMETERS;

	return write_data ("whitelist", params[0].memref.buffer, params[0].memref.size);

	/* Read a key from storage */
	if ( (res = get_objinfo ("key", &objinfo)) != TEE_SUCCESS ) {
		EMSG ("get_objinfo failed, res=0x%08x", res);
		goto cleanup;
	}

	key_size = objinfo.dataSize;
	key_data = TEE_Malloc (key_size, 0);

	if ( (res = read_data ("key", key_data, &key_size)) != TEE_SUCCESS ) {
		EMSG ("Failed to read object, res=0x%08x", res);
		goto cleanup;
	}

	/* decrypt the data and save it to storage as plain text */
	decript_size = params[0].memref.size;
	decript_data = TEE_Malloc (decript_size, 0);
	res = decrypt (key_data, key_size, params[0].memref.buffer, params[0].memref.size, decript_data, &decript_size);
	if (res != TEE_SUCCESS) {
		EMSG ("decript failed, res=0x%08x", res);
		goto cleanup;
	}

	if ( (res = write_data ("whitelist", decript_data, decript_size)) != TEE_SUCCESS ) {
		EMSG ("write_data failed, res=0x%08x", res);
		goto cleanup;
	}

cleanup:
	TEE_Free (key_data);
	TEE_Free (decript_data);
	return res;
}

static TEE_Result tee_wl_verify (uint32_t param_types, TEE_Param params[4]) {
	uint32_t exp_param_types;
	char *p, *r, *tmp, l;
	char path[4096];
	char hash[65];

	/* Check type of parameters */
	exp_param_types = TEE_PARAM_TYPES (
							TEE_PARAM_TYPE_MEMREF_INPUT,
							TEE_PARAM_TYPE_MEMREF_INPUT,
							TEE_PARAM_TYPE_NONE,
							TEE_PARAM_TYPE_NONE);
	if (param_types != exp_param_types) return TEE_ERROR_BAD_PARAMETERS;

	p = wl_data;
	while (1) {
		/* seek at return code */
		if ( (r = strchr (p, '\n')) != NULL ) {
			/* get a line and split it to hash and path */
			l = (r - p) + 1;
			tmp = (char *)TEE_Malloc (sizeof (char) * l, 0);
			strncpy (tmp, p, r-p);
			tmp[l-1] = '\0';
			if ( strsplit (tmp, "\t ", hash, sizeof (hash), path, sizeof (path)) != 0 ) {}
			TEE_Free (tmp);

			DMSG ("%s:%s", path, hash);

			/* verify */
			if ( strncmp (params[0].memref.buffer, path, 4096) == 0 ) {
				if ( strncmp (params[1].memref.buffer, hash, 65) == 0 )
					return TEE_ACCESS_GRANTED;
				else
					return TEE_ERROR_INVALID_HASH;
			}
			p = r + 1;
		} else {
			if ( strsplit (p, "\t ", hash, sizeof (hash), path, sizeof (path)) != 0 ) {}
			DMSG ("last: %s:%s", path, hash);
			/* verify */
			if ( strncmp (params[0].memref.buffer, path, 4096) == 0 ) {
				if ( strncmp (params[1].memref.buffer, hash, 65) == 0 )
					return TEE_ACCESS_GRANTED;
				else
					return TEE_ERROR_INVALID_HASH;
			}
			break;
		}
	}

	return TEE_ERROR_NO_ENTRY;
}

static TEE_Result tee_ml_get_list (uint32_t param_types, TEE_Param params[4]) {
	uint32_t exp_param_types;

	exp_param_types = TEE_PARAM_TYPES (
							TEE_PARAM_TYPE_MEMREF_OUTPUT,
							TEE_PARAM_TYPE_NONE,
							TEE_PARAM_TYPE_NONE,
							TEE_PARAM_TYPE_NONE);

	if (param_types != exp_param_types) return TEE_ERROR_BAD_PARAMETERS;

	return read_data ("marklist", params[0].memref.buffer, &params[0].memref.size);
}

static TEE_Result tee_ml_add (uint32_t param_types, TEE_Param params[4]) {
	uint32_t exp_param_types;

	exp_param_types = TEE_PARAM_TYPES (
							TEE_PARAM_TYPE_MEMREF_INPUT,
							TEE_PARAM_TYPE_NONE,
							TEE_PARAM_TYPE_NONE,
							TEE_PARAM_TYPE_NONE);

	if (param_types != exp_param_types) return TEE_ERROR_BAD_PARAMETERS;

	return write_data ("marklist", params[0].memref.buffer, params[0].memref.size);
}

static TEE_Result tee_key_regist (uint32_t param_types, TEE_Param params[4]) {
	uint32_t exp_param_types;
	TEE_Result res;

	exp_param_types = TEE_PARAM_TYPES (
							TEE_PARAM_TYPE_MEMREF_INPUT,
							TEE_PARAM_TYPE_VALUE_OUTPUT,
							TEE_PARAM_TYPE_NONE,
							TEE_PARAM_TYPE_NONE);

	if (param_types != exp_param_types) return TEE_ERROR_BAD_PARAMETERS;

	if ( (res = write_data ("key", params[0].memref.buffer, params[0].memref.size)) != TEE_SUCCESS )
		EMSG ("write_data failed, res=0x%08x", res);
	else {
		/* Return key id */
		params[1].value.a = 1;
	}

	return res;
}

static TEE_Result tee_key_unregist (uint32_t __unused param_types, TEE_Param __unused params[4]) {
	return TEE_SUCCESS;
}

static TEE_Result tee_key_list (uint32_t __unused param_types, TEE_Param __unused params[4]) {
	list_objid ();
	return TEE_SUCCESS;
}

/*
 * Get object size on secure storage
 * @param (params[0])	object id
 * @param (params[1])	object size
 * @return
*/
static TEE_Result tee_get_objsize (uint32_t param_types, TEE_Param params[4]) {
	uint32_t exp_param_types;
	char id[TEE_OBJECT_ID_MAX_LEN];
	TEE_ObjectInfo objinfo;
	TEE_Result res;

	exp_param_types = TEE_PARAM_TYPES (
							TEE_PARAM_TYPE_MEMREF_INPUT,
							TEE_PARAM_TYPE_VALUE_OUTPUT,
							TEE_PARAM_TYPE_NONE,
							TEE_PARAM_TYPE_NONE);

	if (param_types != exp_param_types) return TEE_ERROR_BAD_PARAMETERS;

	/* Note: The object identifier cannot reside in shared memory. */
	// id = (char *)TEE_Malloc (params[0].memref.size, 0);
	TEE_MemMove (id, params[0].memref.buffer, params[0].memref.size);

	if ( (res = get_objinfo (id, &objinfo)) != TEE_SUCCESS )
		EMSG ("get_objinfo failed, res=0x%08x", res);
	else
		params[1].value.a = objinfo.dataSize;

	return TEE_SUCCESS;
}

static TEE_Result tee_key_gen (uint32_t __unused param_types, TEE_Param __unused params[4]) {
	TEE_Result res;
	// TEE_ObjectInfo objinfo;
	res = generate_keypair ();
	// get_objinfo ("keypair", &objinfo);
	// params[0].memref.buffer = TEE_Malloc (objinfo.dataSize, 0);
	// params[0].memref.size = objinfo.dataSize;
	// read_data ("keypair", params[0].memref.buffer, &params[0].memref.size);
	return res;
}

static TEE_Result tee_key_get_pub (uint32_t __unused param_types, TEE_Param params[4]) {
	TEE_ObjectInfo objinfo;
	TEE_Result res;

	if ( (res = get_objinfo ("keypair", &objinfo)) != TEE_SUCCESS ) {
		return res;
	}

	res = read_data ("keypair", params[0].memref.buffer, &params[0].memref.size);
	return res;
}

// TEE_Result tee_ch_mgtmode (uint32_t param_types, TEE_Param params[4]) {
// 	uint32_t exp_param_types;

// 	exp_param_types = TEE_PARAM_TYPES (
// 								TEE_PARAM_TYPE_NONE,
// 								TEE_PARAM_TYPE_NONE,
// 								TEE_PARAM_TYPE_NONE,
// 								TEE_PARAM_TYPE_NONE);

// 	if (param_types != exp_param_types)
// 		return TEE_ERROR_BAD_PARAMETERS;


// }

// int check_param_types (uint32_t param_types, uint32_t exp_param_types) {

// }

TEE_Result TA_InvokeCommandEntryPoint (void __unused *sess, uint32_t op, uint32_t param_types, TEE_Param params[4]) {
	uint32_t exp_param_types;

	switch (op) {
		case TEE_WL_VERIFY:
			return tee_wl_verify (param_types, params);
		case TEE_ML_GET_LIST:
			return tee_ml_get_list (param_types, params);
		case TEE_KEY_REGIST:
			if (!mgtmode)
				return TEE_ERROR_ACCESS_DENIED;
			return tee_key_regist (param_types, params);
		case TEE_KEY_UNREGIST:
			if (!mgtmode)
				return TEE_ERROR_ACCESS_DENIED;
			return tee_key_unregist (param_types, params);
		case TEE_KEY_LIST:
			if (!mgtmode)
				return TEE_ERROR_ACCESS_DENIED;
			return tee_key_list (param_types, params);
		case TEE_ML_ADD:
			if (!mgtmode)
				return TEE_ERROR_ACCESS_DENIED;
			return tee_ml_add (param_types, params);
		case TEE_GET_OBJSIZE:
			return tee_get_objsize (param_types, params);
		case TEE_WL_APPEND:
			if (!mgtmode)
				return TEE_ERROR_ACCESS_DENIED;
			return tee_wl_append (param_types, params);
		case TEE_WL_INIT:
			return tee_wl_init (param_types, params);
		case TEE_KEY_GEN:
			if (!mgtmode)
				return TEE_ERROR_ACCESS_DENIED;
			return tee_key_gen (param_types, params);
		case TEE_KEY_GET_PUB:
			if (!mgtmode)
				return TEE_ERROR_ACCESS_DENIED;
			return tee_key_get_pub (param_types, params);
		case TEE_CH_MGTMODE:
			exp_param_types = TEE_PARAM_TYPES (
									TEE_PARAM_TYPE_VALUE_INPUT,
									TEE_PARAM_TYPE_NONE,
									TEE_PARAM_TYPE_NONE,
									TEE_PARAM_TYPE_NONE);
			if (param_types != exp_param_types)
				return TEE_ERROR_BAD_PARAMETERS;

			mgtmode = (bool)params[0].value.a;
			return TEE_SUCCESS;
		default:
			EMSG("OP ID 0x%x is not supported", op);
			return TEE_ERROR_NOT_SUPPORTED;
	}
}
