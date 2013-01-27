#ifndef _LOG_H_
#define _LOG_H_

#include <stdarg.h>
#include <stdio.h>

#ifdef __GNUC__
#define CHECK_FMT(a,b) __attribute__((format(printf, a, b)))
#else
#define CHECK_FMT(a,b)
#endif

void log_debug(const char *msg, ...) CHECK_FMT(1,2);
void log_info(const char *msg, ...) CHECK_FMT(1,2);
void log_notice(const char *msg, ...) CHECK_FMT(1,2);
void log_warn(const char *msg, ...) CHECK_FMT(1,2);
void log_error(const char *msg, ...) CHECK_FMT(1,2);
void log_socket_error(const char *msg, ...) CHECK_FMT(1,2);
void log_fatal(const char *msg, ...) CHECK_FMT(1,2);

void log_msg_va(int lvl, int serr, const char *msg, va_list ap);

void log_set_min_level(int);
int log_get_min_level(void);
void log_set_file(FILE *);
void log_set_scrub(int);
int log_get_scrub(void);
const char *log_scrub(const char *);

#endif
