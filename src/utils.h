/*
 * utils.h - Misc utilities
 *
 * Copyright (C) 2013 - 2017, Max Lv <max.c.lv@gmail.com>
 *
 * This file is part of the shadowsocks-libev.
 *
 * shadowsocks-libev is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * shadowsocks-libev is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with shadowsocks-libev; see the file COPYING. If not, see
 * <http://www.gnu.org/licenses/>.
 */

#if defined(USE_CRYPTO_OPENSSL)

#include <openssl/opensslv.h>
#define USING_CRYPTO OPENSSL_VERSION_TEXT

#elif defined(USE_CRYPTO_POLARSSL)
#include <polarssl/version.h>
#define USING_CRYPTO POLARSSL_VERSION_STRING_FULL

#elif defined(USE_CRYPTO_MBEDTLS)
#include <mbedtls/version.h>
#define USING_CRYPTO MBEDTLS_VERSION_STRING_FULL

#endif

#ifndef _UTILS_H
#define _UTILS_H

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#define PORTSTRLEN 16
#define SS_ADDRSTRLEN (INET6_ADDRSTRLEN + PORTSTRLEN + 1)

#ifdef ANDROID

#include <android/log.h>
#define USE_TTY()
#define USE_SYSLOG(ident)
#define LOGI(...)                                                \
    ((void)__android_log_print(ANDROID_LOG_DEBUG, "shadowsocks", \
                               __VA_ARGS__))
#define LOGE(...)                                                \
    ((void)__android_log_print(ANDROID_LOG_ERROR, "shadowsocks", \
                               __VA_ARGS__))

#else // not ANDROID

#define STR(x) # x
#define TOSTR(x) STR(x)

#ifdef LIB_ONLY

extern FILE *logfile;
#define TIME_FORMAT "%Y-%m-%d %H:%M:%S"
#define USE_TTY()
#define USE_SYSLOG(ident)
#define USE_LOGFILE(ident)                                     \
    do {                                                       \
        if (ident != NULL) { logfile = fopen(ident, "w+"); } } \
    while (0)

#define CLOSE_LOGFILE                               \
    do {                                            \
        if (logfile != NULL) { fclose(logfile); } } \
    while (0)
#define LOGI(format, ...)                                                        \
    do {                                                                         \
        if (logfile != NULL) {                                                   \
            time_t now = time(NULL);                                             \
            char timestr[20];                                                    \
            strftime(timestr, 20, TIME_FORMAT, localtime(&now));                 \
            fprintf(logfile, " %s INFO: " format "\n", timestr, ## __VA_ARGS__); \
            fflush(logfile); }                                                   \
    }                                                                            \
    while (0)
#define LOGE(format, ...)                                        \
    do {                                                         \
        if (logfile != NULL) {                                   \
            time_t now = time(NULL);                             \
            char timestr[20];                                    \
            strftime(timestr, 20, TIME_FORMAT, localtime(&now)); \
            fprintf(logfile, " %s ERROR: " format "\n", timestr, \
                    ## __VA_ARGS__);                             \
            fflush(logfile); }                                   \
    }                                                            \
    while (0)

#else // not LIB_ONLY

#include <syslog.h>
extern int use_tty;
extern int use_syslog;

#define HAS_SYSLOG
#define TIME_FORMAT "%F %T"

#define USE_TTY()                        \
    do {                                 \
        use_tty = isatty(STDERR_FILENO); \
    } while (0)

#define USE_SYSLOG(ident)                          \
    do {                                           \
        use_syslog = 1;                            \
        openlog((ident), LOG_CONS | LOG_PID, 0); } \
    while (0)

#define LOGI(format, ...)                                                        \
    do {                                                                         \
        if (use_syslog) {                                                        \
            syslog(LOG_INFO, format, ## __VA_ARGS__);                            \
        } else {                                                                 \
            time_t now = time(NULL);                                             \
            char timestr[20];                                                    \
            strftime(timestr, 20, TIME_FORMAT, localtime(&now));                 \
            if (use_tty) {                                                       \
                fprintf(stderr, "\e[01;32m %s INFO: \e[0m" format "\n", timestr, \
                        ## __VA_ARGS__);                                         \
            } else {                                                             \
                fprintf(stderr, " %s INFO: " format "\n", timestr,               \
                        ## __VA_ARGS__);                                         \
            }                                                                    \
        }                                                                        \
    }                                                                            \
    while (0)

#define LOGE(format, ...)                                                         \
    do {                                                                          \
        if (use_syslog) {                                                         \
            syslog(LOG_ERR, format, ## __VA_ARGS__);                              \
        } else {                                                                  \
            time_t now = time(NULL);                                              \
            char timestr[20];                                                     \
            strftime(timestr, 20, TIME_FORMAT, localtime(&now));                  \
            if (use_tty) {                                                        \
                fprintf(stderr, "\e[01;35m %s ERROR: \e[0m" format "\n", timestr, \
                        ## __VA_ARGS__);                                          \
            } else {                                                              \
                fprintf(stderr, " %s ERROR: " format "\n", timestr,               \
                        ## __VA_ARGS__);                                          \
            }                                                                     \
        } }                                                                       \
    while (0)

#endif // if LIB_ONLY

#endif // if ANDROID

/* Vals for long options */
enum { GETOPT_VAL_HELP = 257,
       GETOPT_VAL_FAST_OPEN,
       GETOPT_VAL_ACL,
       GETOPT_VAL_MTU,
       GETOPT_VAL_MPTCP,
       GETOPT_VAL_PLUGIN,
       GETOPT_VAL_PLUGIN_OPTS,
       GETOPT_VAL_MANAGER_ADDRESS,
       GETOPT_VAL_EXECUTABLE
};

void ERROR(const char *s);

char *ss_itoa(int i);
int ss_isnumeric(const char *s);
int run_as(const char *user);
void FATAL(const char *msg);
void usage(void);
void daemonize(const char *path);
char *ss_strndup(const char *s, size_t n);
#ifdef HAVE_SETRLIMIT
int set_nofile(int nofile);
#endif

void *ss_malloc(size_t size);
void *ss_realloc(void *ptr, size_t new_size);

#define ss_free(ptr)     \
    do {                 \
        free(ptr);       \
        ptr = NULL;      \
    } while (0)

#endif // _UTILS_H

