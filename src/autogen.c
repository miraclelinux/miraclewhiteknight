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
#include <errno.h>
#include <linux/limits.h>
#include <string.h>
#include <openssl/sha.h>
#include <unistd.h>
#include <argp.h>
#include "common.h"
#include "list.h"
#include "file.h"

struct arguments {
	char *chroot;
	char *args[2];
};

int gen_whitelist (const char *chroot_dir, const char *outpath, const char *inpath) {
	FILE *f;
	char path_mnt[PATH_MAX];
	node_t *filelist, *p;
	unsigned char hash[SHA256_DIGEST_LENGTH * 2 + 1];

	filelist = list_init ();

	if ( chroot_dir != NULL && chroot_dir[0] != '\0' ) {
		printf ("Changing root...\n");
		if ( chroot (chroot_dir) ) return -3;
	}

	/* Read list of mount point */
	errno = 0;
	if ( (f = fopen (inpath, "r")) == NULL ) return -1;

	while ( fgets (path_mnt, sizeof (path_mnt), f) != NULL ) {
		chomp (path_mnt);
		printf ("Searching executable files in %s\n", path_mnt);
		if ( get_files (filelist , path_mnt, 1) ) return -1;
	}

	fclose (f);

	/* generate file hash */
	if ( (f = fopen (outpath, "w")) == NULL ) return -2;

	for (p = filelist->next; !list_is_tail (p); p = p->next) {
		if ( is_exec ((char *)p->data) ) {
			get_filehash ((char *)p->data, hash);
			fprintf (f, "%s %s\n", hash, (char *)p->data);
		}
	}

	fclose (f);

	return 0;
}

static error_t parse_opt (int key, char *arg, struct argp_state *state) {
	struct arguments *arguments = state->input;

	switch (key) {
		case 'c':
			arguments->chroot = arg;
			break;

		case ARGP_KEY_ARG:
			if (state->arg_num >= 2)
				/* Too many arguments. */
				argp_usage (state);
			arguments->args[state->arg_num] = arg;
			break;

		case ARGP_KEY_END:
			if (state->arg_num < 2)
				/* Not enough arguments. */
				argp_usage (state);
			break;

		case ARGP_KEY_NO_ARGS:
			argp_usage (state);
			break;

		default:
			return ARGP_ERR_UNKNOWN;
	}
	return 0;
}


int main (int argc, char *argv[]) {
	int err;
	const char doc[] = "A program to generate whitelists for whiteknight.";
	const char args_doc[] = "[output list file] [input target file]";
	struct argp_option options[] = {
		{ "chroot", 'c', "Directory", 0, "Change the root directory" },
		{ 0 }
	};
	struct argp argp = { options, parse_opt, args_doc, doc };

	struct arguments arguments;

	argp_parse (&argp, argc, argv, 0, 0, &arguments);

	err = gen_whitelist (arguments.chroot, arguments.args[0], arguments.args[1]);
	if ( err == -1 ) {
		perror (argv[1]);
		exit (EXIT_FAILURE);
	} else if ( err == -2 ) {
		perror (argv[1]);
		exit (EXIT_FAILURE);
	} else if ( err == -3 ) {
		perror (arguments.chroot);
		exit (EXIT_FAILURE);
	}

	exit (EXIT_SUCCESS);
}
