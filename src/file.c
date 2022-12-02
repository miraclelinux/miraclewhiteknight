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
#include <errno.h>
#include <dirent.h>
#include <string.h>
#include <linux/limits.h>
#include <sys/stat.h>
#include <openssl/sha.h>
#include <fcntl.h>
#include <unistd.h>
#include "file.h"
#include "list.h"

int is_exec (const char *path) {
	struct stat st;

	if ( stat (path, &st) == -1 ) return -1;
	return st.st_mode & (S_IXUSR | S_IXGRP | S_IXOTH);
}

int is_exec_from_fd (int fd) {
	struct stat st;

	if ( fstat(fd, &st) == -1 ) return -1;

	return st.st_mode & (S_IXUSR | S_IXGRP | S_IXOTH);
}

int get_files (node_t *list, const char *dirpath, int recursive) {
	DIR *d;
	struct dirent *dp;
	char path[PATH_MAX];

	errno = 0;
	if ( (d = opendir (dirpath)) == NULL ) return -1;

	while ( (dp = readdir (d)) != NULL ) {
		if ( strncmp (dp->d_name, ".", 1) != 0 && strncmp (dp->d_name, "..", 2) != 0 ) {
			snprintf (path, sizeof (path) - 1, "%s/%s", dirpath, dp->d_name);
			if ( dp->d_type == DT_REG || dp->d_type == DT_LNK ) {
				if (list_append (list, path, sizeof (path)) == -1) return -1;
			} else if ( dp->d_type == DT_DIR && recursive ) {
				get_files (list, path, recursive);
			}
		}
	}

	return 0;
}

int get_filehash (const char *filepath, unsigned char* hash) {
	int fd;
	int size;
	int i;
	SHA256_CTX ctx;
	unsigned char buf[READSIZE];
	unsigned char _hash[SHA256_DIGEST_LENGTH];

	SHA256_Init (&ctx);

	if ( (fd = open (filepath, O_RDONLY)) == -1 ) return -1;

	while ( (size = read (fd, buf, READSIZE)) != 0 )
		SHA256_Update (&ctx, buf, size);

	close (fd);
	SHA256_Final (_hash, &ctx);

	/* convert hex string */
	for (i = 0; i < SHA256_DIGEST_LENGTH; i++)
		sprintf ((char *)(hash + (i * 2)), "%02x", _hash[i]);
	return 0;
}

/*
 * Get file hash from fd
 * @param (fd)
 * @param (hash)
 * @return: always return 0
*/
int get_filehash_from_fd (int fd, unsigned char *hash) {
	int size;
	int i;
	SHA256_CTX ctx;
	unsigned char buf[READSIZE];
	unsigned char _hash[SHA256_DIGEST_LENGTH];

	SHA256_Init (&ctx);

	while ( (size = read (fd, buf, READSIZE)) != 0 )
		SHA256_Update (&ctx, buf, size);

	SHA256_Final (_hash, &ctx);

	/* convert hex string */
	for (i = 0; i < SHA256_DIGEST_LENGTH; i++)
		sprintf ((char *)(hash + (i * 2)), "%02x", _hash[i]);
	return 0;
}

/*
 * Get file extention
 * @param (filename): filename
 * @return: return file extension if got, otherwise return NULL;
 */
const char *get_extension (const char *filename) {
	const char *dot = strrchr (filename, '.');
	if ( !dot || dot == filename ) return NULL;
	return dot + 1;
}

/*
 * Get file path from file descriptor
 * @param (path)
 * @param (len)
 * @param (fd)
 * @return: return -1 if errors occured. 0 is fine.
*/
int get_path_from_fd (char *path, size_t len, int fd) {
	ssize_t path_len;
	char fdpath[PATH_MAX];

	snprintf (fdpath, sizeof(fdpath) - 1, "/proc/self/fd/%d", fd);
	path_len = readlink (fdpath, path, len - 1);

	if (path_len == -1) return -1;

	path[path_len] = '\0';
	return 0;
}
