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
#define _GNU_SOURCE
#include "common.h"
#include "init.h"
#include "list.h"
#include "log.h"
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/fanotify.h>
#include <fcntl.h>
#include <errno.h>

int get_fan_fd(){
    int fan_fd;
    fan_fd = fanotify_init(FAN_CLOEXEC | FAN_CLASS_CONTENT | FAN_NONBLOCK,
                           O_RDONLY | O_LARGEFILE);

    if(fan_fd == -1){
        log_err ("%s: %s", "fanotify init", strerror (errno));
        exit(EXIT_FAILURE);
    }

    return fan_fd;
}

/*
 * Add mark from list
 * @param (fd)      file description of fanotify
 * @param (*list)   list of mount path
 * @return          return size if success. Otherwise, -1 shall be returned and errno set to indicate the error.
*/
int fan_mark_from_list (int fd, node_t *list) {
    node_t *p;
    int res;

    for ( p = list->next; !list_is_tail (p); p = p->next ) {
        log_debug ("mark: %s", (char *)p->data);
        res = fanotify_mark (
                        fd,
                        FAN_MARK_ADD|FAN_MARK_MOUNT,
                        FAN_OPEN_PERM,
                        AT_FDCWD,
                        (char *)p->data);
        if (res == -1) {
            log_err ("%s: %s", __FUNCTION__, strerror (errno));
            return -1;
        }
    }
    return 0;
}

int fan_mark(int fan_fd, const char *filepath){
    FILE *f;
    char mntpath[PATH_MAX];

    if ( (f = fopen(filepath, "r")) == NULL ) return -1;

    while ( fscanf (f, "%4095s", mntpath) != EOF ) {
        log_debug ("mark: %s", mntpath);
        if(fanotify_mark(fan_fd, FAN_MARK_ADD | FAN_MARK_MOUNT,
                         FAN_OPEN_PERM, AT_FDCWD, mntpath) == -1) return -1;
    }

    fclose (f);

    return 0;
}

void fan_add_whitelist(const char *filepath){
    FILE *f;
    wl_entry_t e;

    if ( (f = fopen(filepath, "r")) == NULL ) {
        log_err (strerror (errno));
        return;
    }

    /* This code may occur buffer overflow */
    while ( fscanf (f, "%64s %4095s", e.hash, e.filepath) != EOF ) {
        log_debug ("add: %s (%s)", e.filepath, e.hash);
        list_append (whitelist, &e, sizeof (e));
    }

    fclose (f);
}

