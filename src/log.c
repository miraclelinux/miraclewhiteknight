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
#include "log.h"
#include <string.h>
#include <time.h>
#include <stdarg.h>
#include "config.h"

#ifdef CONFIG_SYSLOG
#include <syslog.h>
#elif defined CONFIG_SYSTEMD
#include <syslog.h>
#include <systemd/sd-journal.h>
#else
#include <stdio.h>
#endif

void get_datestr (char *datestr, int size) {
    time_t now = time (NULL);
    strftime (datestr, size, "%Y-%m-%d %H:%M:%S", localtime (&now));
}

void log_err (const char *format, ...) {
    va_list ap;

#if !defined CONFIG_SYSTEMD && !defined CONFIG_SYSLOG
    char datestr[256];
    get_datestr (datestr, sizeof (datestr));
    printf ("\033[031;1m[Error][%s]\033[0;1m ", datestr);
#endif

    va_start (ap, format);
#ifdef CONFIG_SYSLOG
    vsyslog (LOG_ERR, format, ap);
#elif defined CONFIG_SYSTEMD
    sd_journal_printv (LOG_ERR, format, ap);
#else
    vprintf (format, ap);
    printf ("\n\033[0m");
#endif
    va_end (ap);
}

void log_info (const char *format, ...) {
    va_list ap;

#if !defined CONFIG_SYSLOG && !defined CONFIG_SYSTEMD
    char datestr[256];
    get_datestr (datestr, sizeof (datestr));
    printf ("\033[1m[Info ][%s]\033[0m ", datestr);
#endif

    va_start (ap, format);
#ifdef CONFIG_SYSLOG
    vsyslog (LOG_INFO, format, ap);
#elif defined CONFIG_SYSTEMD
    sd_journal_printv (LOG_INFO, format, ap);
#else
    vprintf (format, ap);
    printf ("\n\033[0m");
#endif
    va_end (ap);
}

void log_warn (const char *format, ...) {
    va_list ap;

#if !defined CONFIG_SYSLOG && !defined CONFIG_SYSTEMD
    char datestr[256];
    get_datestr (datestr, sizeof (datestr));
    printf ("\033[033;1m[Warn ][%s]\033[0;1m ", datestr);
 #endif

    va_start (ap, format);
#ifdef CONFIG_SYSLOG
    vsyslog (LOG_WARNING, format, ap);
#elif defined CONFIG_SYSTEMD
    sd_journal_printv (LOG_WARNING, format, ap);
#else
    vprintf (format, ap);
    printf ("\n\033[0m");
#endif
    va_end (ap);
}


void log_debug (const char *format, ...) {
#ifdef DEBUG
    va_list ap;

#if !defined CONFIG_SYSLOG && !defined CONFIG_SYSTEMD
    char datestr[256];
    get_datestr (datestr, sizeof (datestr));
    printf ("\033[1m[Debug][%s]\033[0m ", datestr);
 #endif

    va_start (ap, format);
#ifdef CONFIG_SYSLOG
    vsyslog (LOG_DEBUG, format, ap);
#elif defined CONFIG_SYSTEMD
    sd_journal_printv (LOG_DEBUG, format, ap);
#else
    vprintf (format, ap);
    printf ("\n\033[0m");
#endif
    va_end (ap);
#endif
}
