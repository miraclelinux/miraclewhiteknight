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
#include "object.h"
#include "key.h"

TEE_Result aes_decrypt () {
	return TEE_ERROR_NOT_IMPLEMENTED;
}

TEE_Result decrypt (void *key, unsigned int key_size, void *src_data, unsigned int src_size, void *dst_data, unsigned int *dst_size) {
	TEE_OperationHandle ophdl;
	TEE_ObjectHandle objhdl;
	TEE_Result res;
	TEE_Attribute attr;

	attr.attributeID = TEE_ATTR_RSA_PUBLIC_EXPONENT;
	attr.content.ref.buffer = key;
	attr.content.ref.length = key_size;

	if ( (res = TEE_AllocateTransientObject (TEE_TYPE_RSA_KEYPAIR, RSA_KEY_BIT, &objhdl)) != TEE_SUCCESS ) {
		EMSG ("TEE_AllocateTransientObject failed res=0x%08x", res);
		goto cleanup;
	}

	if ( (res = TEE_PopulateTransientObject (objhdl, &attr, 1)) != TEE_SUCCESS ) {
		EMSG ("TEE_PopulateTransientObject failed res=0x%08x", res);
		goto cleanup;
	}

	if ( (res = TEE_AllocateOperation (&ophdl, TEE_ALG_RSAES_PKCS1_V1_5, TEE_MODE_DECRYPT, RSA_KEY_BIT)) != TEE_SUCCESS ) {
		EMSG ("TEE_AllocateOperation failed res=0x%08x", res);
		goto cleanup;
	}

	if ( (res = TEE_SetOperationKey (ophdl, objhdl)) != TEE_SUCCESS ) {
		EMSG ("TEE_AllocateOperation failed res=0x%08x", res);
		goto cleanup;
	}

	if ( (res = TEE_AsymmetricDecrypt (ophdl, (TEE_Attribute *)NULL, 0, src_data, src_size, dst_data, dst_size)) != TEE_SUCCESS ) {
		EMSG ("TEE_AsymmetricDecrypt failed res=0x%08x", res);
		goto cleanup;
	}

cleanup:
	TEE_FreeOperation (ophdl);
	TEE_FreeTransientObject (objhdl);
	return res;
}

TEE_Result generate_keypair (void) {
	TEE_ObjectHandle keypair;
	// TEE_ObjectInfo objinfo;
	TEE_Result res;
	// void *data;
	// uint32_t data_size;
	// uint32_t flag;

	res = TEE_AllocateTransientObject (TEE_TYPE_RSA_KEYPAIR, RSA_KEY_BIT, &keypair);
	if ( res != TEE_SUCCESS ) {
		return res;
	}

	res = TEE_GenerateKey (keypair, RSA_KEY_BIT, NULL, 0);
	if ( res != TEE_SUCCESS ) {
		goto cleanup;
	}

	// flag = TEE_DATA_FLAG_ACCESS_WRITE|TEE_DATA_FLAG_OVERWRITE;
	// res = TEE_CreatePersistentObject (TEE_STORAGE_PRIVATE, "key", 3, flag, keypair, NULL, 0, &objhdl);
	// if ( res != TEE_SUCCESS ) {
	// 	EMSG ("TEE_CreatePersistentObject failed res=0x%08x", res);
	// 	goto cleanup;
	// }

	// res = TEE_GetObjectInfo1 (objhdl, &objinfo);
	// if ( res != TEE_SUCCESS ) {
	// 	EMSG ("TEE_GetObjectInfo1 failed res=0x%08x", res);
	// 	goto cleanup;
	// }

	// res = TEE_WriteObjectData (objhdl, )
	// res = TEE_GetObjectInfo1 (keypair, &objinfo);
	// if ( res != TEE_SUCCESS ) {
		// goto cleanup;
	// }

	// data_size = objinfo.objectSize;
	// DMSG ("size: %d", objinfo.objectSize);

	// data_size = objinfo.dataSize;
	// data = TEE_Malloc (data_size, 0);
	// res = TEE_ReadObjectData (objhdl, data, objinfo.dataSize, &data_size);
	// if ( res != TEE_SUCCESS ) {
	// 	EMSG ("TEE_ReadObjectData failed res=0x%08x", res);
	// 	goto cleanup;
	// }

	// DMSG ("size: %d", data_size);

	// EMSG ("size: %d", objinfo.keySize);
	// EMSG ("size: %d", objinfo.dataSize);

	// data = TEE_Malloc (data_size, 0);

	uint8_t pubexp[RSA_KEY_BIT];
	uint32_t pubexp_size = RSA_KEY_BIT;
	res = TEE_GetObjectBufferAttribute (keypair, TEE_ATTR_RSA_PUBLIC_EXPONENT, pubexp, &pubexp_size);
	if (res != TEE_SUCCESS) {
		EMSG ("TEE_GetObjectBufferAttribute failed res = 0x%08x", res);
		goto cleanup;
	}

	uint8_t mod[RSA_KEY_BIT];
	uint32_t mod_size = RSA_KEY_BIT;
	res = TEE_GetObjectBufferAttribute (keypair, TEE_ATTR_RSA_MODULUS, mod, &mod_size);
	if (res != TEE_SUCCESS) {
		EMSG ("TEE_GetObjectBufferAttribute failed res = 0x%08x", res);
		goto cleanup;
	}

	TEE_BigInt *tmp_pubexp, *tmp_mod;
	size_t bigint_size;
	bigint_size = TEE_BigIntSizeInU32 (RSA_KEY_BIT);

	tmp_pubexp = (TEE_BigInt *)TEE_Malloc (bigint_size * sizeof (TEE_BigInt), 0);
	tmp_mod = (TEE_BigInt *)TEE_Malloc (bigint_size * sizeof (TEE_BigInt), 0);

	TEE_BigIntInit (tmp_pubexp, RSA_KEY_BIT);
	TEE_BigIntInit (tmp_mod, RSA_KEY_BIT);


	res = TEE_BigIntConvertFromOctetString (tmp_pubexp, pubexp, pubexp_size, 0);
	if (res != TEE_SUCCESS) {
		EMSG ("TEE_BigIntConvertFromOctetString failed res = 0x%08x", res);
		goto cleanup;
	}
	res = TEE_BigIntConvertFromOctetString (tmp_mod, mod, mod_size, 0);
	if (res != TEE_SUCCESS) {
		EMSG ("TEE_BigIntConvertFromOctetString failed res = 0x%08x", res);
		goto cleanup;
	}
	// EMSG ("%x %x", *tmp_pubexp, *tmp_mod);
	// EMSG ("%hhn %hhn", pubexp, mod);
	// EMSG ("%x %d", pubexp, pubexp_size);



	// res = write_data ("keypair", data, data_size);
	// res = write_data ("keypair", keypair, objinfo.dataSize);
cleanup:
	// TEE_Free (data);
	TEE_FreeTransientObject (keypair);
	return res;
}
