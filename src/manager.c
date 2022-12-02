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
#include <argp.h>
#include <stdlib.h>
#include <string.h>
#include <tee_client_api.h>
#include "tee.h"
#include "log.h"
#include "ta/user_ta_header_defines.h"

enum operation {
	NONE,
	ADD_WL,
	ADD_ML,
	REGIST_KEY,
	UNREGIST_KEY,
	LIST_KEYS,
	GEN_KEYPAIR,
};

struct args {
	char *id;
	enum operation op;
	char *filename;
};

typedef struct args args_t;

error_t parse_opt (int key, char *arg, struct argp_state *state) {
	args_t *args = state->input;

	switch (key) {
		case 'A':
			args->op = ADD_WL;
			args->filename = arg;
			break;
		case 'a':
			args->op = ADD_ML;
			args->filename = arg;
			break;
		case 'U':
			printf (arg);
			args->op = UNREGIST_KEY;
			args->id = arg;
			break;
		case 'R':
			args->op = REGIST_KEY;
			args->filename = arg;
			break;
		case 'L':
			args->op = LIST_KEYS;
			break;
		case 'G':
			args->op = GEN_KEYPAIR;
			break;
		// case ARGP_KEY_NO_ARGS:
		// 	argp_usage (state);
		default:
			return ARGP_ERR_UNKNOWN;
	}

	return 0;
}

int main (int argc, char *argv[]) {
	TEEC_Context ctx;
	TEEC_Session sess;
	TEEC_Result res;
	TEEC_UUID uuid = TA_UUID;
	uint32_t rorig;
	uint32_t keyid;

	const char doc[] = "Manager for whiteknight.";
	const char args_doc[] = "";
	struct argp_option options[] = {
		{ 0, 0, 0, 0, "Key management" },
		{ "resist-key", 'R', "PATH", 0, "resist a key to tee" },
		{ "unresist-key", 'U', "ID", 0, "unresist the key from tee" },
		{ "list-keys", 'L', 0, 0, "list keys on tee" },
		{ "gen-keypair", 'G', 0, 0, "generate key pair on tee" },

		{ 0, 0, 0, 0, "Whitelist management" },
		{ "add-wl", 'A', "PATH", 0, "add whitelist to tee" },

		{ 0, 0, 0, 0, "Marklist management" },
		{ "add-ml", 'a', "PATH", 0, "add marklist to tee" },

		{ 0 }
	};
	struct argp argp = { options, parse_opt, args_doc, doc };
	args_t args = {};

	if ( argp_parse (&argp, argc, argv, ARGP_LONG_ONLY, 0, &args) != 0 )
		exit (EXIT_FAILURE);

	log_info ("tee: initializing context");
	res = TEEC_InitializeContext (NULL, &ctx);
	if (res != TEEC_SUCCESS) {
		log_err ("tee: failed to initialize context");
		exit (EXIT_FAILURE);
	}

	log_info ("tee: opening session");
	res = TEEC_OpenSession (&ctx, &sess, &uuid, TEEC_LOGIN_PUBLIC, NULL, NULL, &rorig);

	if (res != TEEC_SUCCESS) {
		log_err ("tee: session open error");
		exit (EXIT_FAILURE);
	}

	if ( tee_change_mgtmode (&sess, true) != 0 ) {
		log_err ("Mode change failed.");
		exit (EXIT_FAILURE);
	}

	switch (args.op) {
		case ADD_ML:
			tee_ml_add (&sess, args.filename);
			break;
		case ADD_WL:
			tee_wl_append (&sess, args.filename);
			break;
		case REGIST_KEY:
			if ( tee_key_regist (&sess, args.filename, &keyid) != 0 ) {
				log_err ("%s: %s", strerror (errno), args.filename);
			}
			log_info ("keyid: %d", keyid);
			break;
		case UNREGIST_KEY:
			break;
		case LIST_KEYS:
			if ( tee_key_list (&sess) ) {}
			break;
		case GEN_KEYPAIR:
			log_info ("tee: generating a RSA key pair...");
			tee_key_gen (&sess);
			break;
		default:
			break;
	}

	if ( tee_change_mgtmode (&sess, false) != 0 ) {
		log_err ("Mode change failed.");
		exit (EXIT_FAILURE);
	}

	TEEC_CloseSession (&sess);

	return 0;
}
