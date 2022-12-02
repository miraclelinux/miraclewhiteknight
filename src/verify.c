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
#include "common.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <linux/limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdbool.h>
#include "verify.h"
#include "log.h"
#include "file.h"
#include "config.h"

#ifdef CONFIG_TEE
#include <tee_client_api.h>
#include "tee.h"
#include "ta/user_ta_header_defines.h"
#endif

/*
 * Verify the file is correct by whitelist
 * @param (fd)
 * @return: return 1 if the file is valid. return 0 if the file is invalid. otherwise return -1;
*/

int verify (int fd) {
    char filepath[PATH_MAX];
	/* TODO: define hash length */
	unsigned char filehash[65];
	int res;

#ifndef CONFIG_TEE
	bool valid_path = false;
	bool valid_hash = false;
    node_t *n;
#endif
    /* Get filepath, then get its hash if the file is executable */
    get_path_from_fd (filepath, sizeof (filepath), fd);

    /* It is normal file or directory if the file is not executable.
     * it should be allowed.
    */
    if ( is_exec_from_fd (fd) == 0 ) {
        log_debug ("non executable file: %s", filepath);
        return 1;
    }

    if ( get_filehash_from_fd (fd, filehash) != 0 ) {
		log_warn ("%s: %s", __FUNCTION__, strerror (errno));
		return -1;
    }

#ifdef CONFIG_TEE
    res = tee_wl_verify (&sess, filepath, filehash);
#else
	for (n = whitelist->next; !list_is_tail (n); n = n->next) {
		/* Check the paths between in the whitelist and given. After that, check the hashes like before. */
		if ( strncmp (filepath, ((wl_entry_t *)n->data)->filepath, PATH_MAX) == 0 ) {
			valid_path = true;
			/* TODO: define hash lengh */
			if ( strncmp ((char *)filehash, (char *)((wl_entry_t *)n->data)->hash, 65) == 0 )
				valid_hash = true;
			break;
		}
	}

	if ( valid_path && valid_hash ) {
		res = 0;
    } else if ( !valid_path && !valid_hash ) {
		res = -1;
	} else if ( valid_path && !valid_hash ) {
		res = -1;
	}
#endif
	if ( res == 0 ) {
        log_info ("%s: accept: %s", __FUNCTION__, filepath);
        return 1;
    } else {
		log_err ("%s: invalid hash: %s", __FUNCTION__, filepath);
        log_err ("%s: invalid path: %s", __FUNCTION__, filepath);
		return 0;
	}
}
