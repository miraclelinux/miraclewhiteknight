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
#include <errno.h>
#include <string.h>
#include <sys/fanotify.h>
#include <unistd.h>
#include <stdlib.h>
#include "log.h"
#include "handle.h"
#include "verify.h"

void handle_events(int fan_fd){
    const struct fanotify_event_metadata *metadata;
    struct fanotify_event_metadata buf[200];
    ssize_t len;
    struct fanotify_response response;

    while(1){
        len = read(fan_fd, (void *) &buf, sizeof(buf));
        if(len == -1 && errno != EAGAIN){
            log_err ("%s: %s", "handler", strerror (errno));
            exit(EXIT_FAILURE);
        }

        if(len <= 0){
            break;
        }

        metadata = buf;

        while(FAN_EVENT_OK(metadata, len)){
            if(metadata->vers != FANOTIFY_METADATA_VERSION){
                log_err ("handler: Mismatch fan metadata vers.");
                exit(EXIT_FAILURE);
            }

            if(metadata->fd >= 0){
                if(verify(metadata->fd) == 1 || debug == true){
                    if (debug)
                        log_warn ("%s: %s", __FUNCTION__, "debug: execution allowed");
                    response.response = FAN_ALLOW;
                }else{
                    response.response = FAN_DENY;
                }

                response.fd = metadata->fd;
                write(fan_fd, &response, sizeof(struct fanotify_response));

                close(metadata->fd);
            }

            metadata = FAN_EVENT_NEXT(metadata, len);
        }
    }
}
