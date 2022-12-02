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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <linux/limits.h>
#include <dirent.h>
#include <errno.h>
#include "init.h"
#include "handle.h"
#include "log.h"
#include "file.h"
#include "list.h"
#include "config.h"
#include "common.h"
#include <argp.h>

#ifdef CONFIG_SYSLOG
#include <syslog.h>
#endif

#ifdef CONFIG_TEE
#include <tee_client_api.h>
#include "tee.h"
#include "ta/user_ta_header_defines.h"
#endif

node_t *whitelist = NULL;
bool debug = false;

struct arguments {
    char *chroot;
    char *args[2];
};

static error_t parse_opt (int key, char *arg, struct argp_state *state) {
    switch (key) {
        case 'd':
            debug = true;
            break;

        default:
            return ARGP_ERR_UNKNOWN;
    }
    return 0;
}


int main(int argc, char *argv[]){
    int fan_fd;

    /* parser data */
    const char doc[] = "Whitelist driven mandatory access control program";
    const char args_doc[] = "";
    struct argp_option options[] = {
        { "debug", 'd', NULL, 0, "Run in debug mode" },
        { 0 }
    };
    struct argp argp = { options, parse_opt, args_doc, doc };
    struct arguments arguments;

#ifdef CONFIG_SYSLOG
    openlog (PACKAGE_NAME, LOG_CONS|LOG_PID|LOG_PERROR, LOG_USER);
#endif

    /* parse arguments */
    argp_parse (&argp, argc, argv, 0, 0, &arguments);

    if (debug)
        log_warn ("debug mode is enabled");

    log_debug ("white-knight up");

    fan_fd = get_fan_fd();

#ifdef CONFIG_TEE
    TEEC_Result res;
    TEEC_UUID uuid = TA_UUID;
    uint32_t rorig;
    node_t *mplist;

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

    log_info ("tee: initializing whitelist...");
    if ( tee_wl_init (&sess) != 0 ) {
        log_err ("tee: failed to initialize whitelist");
        exit (EXIT_FAILURE);
    }

    log_info ("tee: loading mark lists...");
    mplist = list_init();
    if ( tee_ml_get_list (&sess, mplist) != 0 ) {
        log_err ("tee: cannot load mark lists");
        exit (EXIT_FAILURE);
    }

    if ( fan_mark_from_list (fan_fd, mplist) != 0 ) {
        log_err (strerror (errno));
        exit (EXIT_FAILURE);
    }

    log_info ("tee: ok");
#else
    char targets_dir_path[PATH_MAX];
    char list_dir_path[PATH_MAX];
    node_t *filelist, *p, *tmp;

    whitelist = list_init();

    /* Load whiltelist */
    tmp = list_init ();

    snprintf (list_dir_path, sizeof (list_dir_path) - 1, "%s/%s/%s", DATAROOTDIR, PACKAGE_NAME, LIST_DIR);
    if ( get_files (tmp, list_dir_path, 0) )
        log_info ("%s: %s", strerror (errno), list_dir_path);

    snprintf (list_dir_path, sizeof (list_dir_path) - 1, "%s/%s/%s", CONFIGDIR, PACKAGE_NAME, LIST_DIR);
    if ( get_files (tmp, list_dir_path, 0) )
        log_info ("%s: %s", strerror (errno), list_dir_path);

    for ( p = tmp->next; !list_is_tail (p); p = p->next) {
        log_info ("loading rule: %s", (char *)p->data);
        /* TODO: This function occur segfault when file has empty line. */
        fan_add_whitelist ((char *)p->data);
    }

    list_free (tmp);

    /* Load target list */
    filelist = list_init();

    snprintf (targets_dir_path, sizeof (targets_dir_path) - 1, "%s/%s/%s", DATAROOTDIR, PACKAGE_NAME, TARGETS_DIR);

    if (get_files (filelist, targets_dir_path, 0))
        log_info ("%s: %s", strerror (errno), targets_dir_path);

    snprintf (targets_dir_path, sizeof (targets_dir_path) - 1, "%s/%s/%s", CONFIGDIR, PACKAGE_NAME, TARGETS_DIR);
    if ( get_files (filelist, targets_dir_path, 0) )
        log_info ("%s: %s", strerror (errno), targets_dir_path);

    for ( p = filelist->next; !list_is_tail (p); p = p->next ) {
        log_info ("loading targets: %s", (char *)p->data);
        fan_mark (fan_fd, (char *)p->data);
    }

    list_free (filelist);

    if ( list_is_empty (whitelist) ) {
        log_err ("whitelist is empty.");
        exit (EXIT_FAILURE);
    }
#endif

    while(1){
        handle_events(fan_fd);
    }

    exit(EXIT_SUCCESS);
}
