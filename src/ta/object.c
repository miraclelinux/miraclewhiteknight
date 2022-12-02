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
#include <tee_internal_api.h>
#include <string.h>
#include "object.h"

TEE_Result list_objid (void) {
	TEE_ObjectEnumHandle hdl;
	TEE_ObjectInfo objinfo;
	TEE_Result res = TEE_SUCCESS;
	char objid[TEE_OBJECT_ID_MAX_LEN];
	uint32_t objid_size;

	if ( TEE_AllocatePersistentObjectEnumerator (&hdl) != TEE_SUCCESS ) {
		EMSG ("TEE_AllocatePersistentObjectEnumerator failed res=0x%08x", res);
		return res;
	}

	if ( (res = TEE_StartPersistentObjectEnumerator (hdl, TEE_STORAGE_PRIVATE)) != TEE_SUCCESS ) {
		EMSG ("TEE_StartPersistentObjectEnumerator failed res=0x%08x", res);
		goto cleanup;
	}

	while (1) {
		objid_size = TEE_OBJECT_ID_MAX_LEN;
		res = TEE_GetNextPersistentObject (hdl, &objinfo, objid, &objid_size);
		if ( res == TEE_SUCCESS ) {
			IMSG ("ID: %s", objid);
		} else if (res == TEE_ERROR_ITEM_NOT_FOUND) {
			break;
		} else {
			EMSG ("TEE_GetNextPersistentObject failed res=0x%08x", res);
			break;
		}
	}

cleanup:
	TEE_FreePersistentObjectEnumerator (hdl);
	return res;
}

/*
 * Save data
 * @param (*id)		object id
 * @param (*data)	pointer of data
 * @param (size)	data size
 * @return
*/
TEE_Result get_objinfo (const char *id, TEE_ObjectInfo *objinfo) {
	TEE_Result res;
	TEE_ObjectHandle objhdl;
	uint32_t flag;

	flag = TEE_DATA_FLAG_ACCESS_READ;
	if ( (res = TEE_OpenPersistentObject (TEE_STORAGE_PRIVATE, id, strlen (id), flag, &objhdl)) != TEE_SUCCESS ) {
		EMSG ("Failed to open persistent object, res=0x%08x", res);
		return res;
	}

	if ( (res = TEE_GetObjectInfo1 (objhdl, objinfo)) != TEE_SUCCESS ) {
		EMSG ("Failed to get object info, res=0x%08x", res);
		goto cleanup;
	}

cleanup:
	TEE_CloseObject (objhdl);
	return res;
}

/*
 * Save data
 * @param (*id)		object id
 * @param (*data)	pointer of data
 * @param (size)	data size
 * @return
*/
TEE_Result write_data (const char *id, void *data, uint32_t size) {
	uint32_t flag;
	TEE_ObjectHandle obj;
	TEE_Result res;

	flag = TEE_DATA_FLAG_ACCESS_READ | TEE_DATA_FLAG_ACCESS_WRITE | TEE_DATA_FLAG_ACCESS_WRITE_META | TEE_DATA_FLAG_OVERWRITE;

	/* TODO: change storage */
	res = TEE_CreatePersistentObject (TEE_STORAGE_PRIVATE, id, strlen (id), flag, TEE_HANDLE_NULL, NULL, 0, &obj);

	if (res != TEE_SUCCESS) {
		EMSG ("TEE_CreatePersistentObject failed 0x%08x", res);
		return res;
	}

	res = TEE_WriteObjectData (obj, data, size);

	if (res != TEE_SUCCESS) {
		EMSG ("TEE_WriteObjectData failed 0x%08x", res);
		res = TEE_CloseAndDeletePersistentObject1 (obj);
		if (res != TEE_SUCCESS)
			EMSG ("TEE_CloseAndDeletePersistentObject1 failed 0x%08x", res);
	} else
		TEE_CloseObject (obj);
	return res;
}

/*
 * Save data
 * @param (*id)		object id
 * @param (*data)	pointer of data
 * @param (*size)	refered data size
 * @return
*/
TEE_Result read_data (const char *id, void *data, uint32_t *size) {
	uint32_t flag;
	TEE_ObjectHandle obj;
	TEE_ObjectInfo obj_info;
	TEE_Result res;
	uint32_t rd_size;

	flag = TEE_DATA_FLAG_ACCESS_READ | TEE_DATA_FLAG_SHARE_READ;
	res = TEE_OpenPersistentObject (TEE_STORAGE_PRIVATE, id, strlen (id), flag, &obj);
	if (res != TEE_SUCCESS) {
		EMSG ("Failed to open persistent object, res=0x%08x", res);
		return res;
	}

	if ( (res = TEE_GetObjectInfo1 (obj, &obj_info)) != TEE_SUCCESS ) {
		EMSG ("Failed to create persistent object, res=0x%08x", res);
		goto cleanup;
	}

	if (obj_info.dataSize > *size) {
		res = TEE_ERROR_SHORT_BUFFER;
		goto cleanup;
	}

	res = TEE_ReadObjectData (obj, data, obj_info.dataSize, &rd_size);
	if (res != TEE_SUCCESS || rd_size != obj_info.dataSize) {
		EMSG ("TEE_ReadObjectData failed 0x%08x, read %" PRIu32 " over %u", res, rd_size, obj_info.dataSize);
		goto cleanup;
	}
	*size = rd_size;

cleanup:
	TEE_CloseObject (obj);
	return res;
}
