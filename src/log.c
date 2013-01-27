#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <stdarg.h>
#include "log.h"
#include "util.h"

static int min_log_level = LOG_NOTICE;
static FILE *log_file = NULL;
static int log_do_scrub = 1;

extern int debug;	/* from main.c */

void
log_debug(const char *msg, ...)
{
	va_list ap;

	va_start(ap, msg);
	log_msg_va(LOG_DEBUG, 0, msg, ap);
	va_end(ap);
}

void
log_info(const char *msg, ...)
{
	va_list ap;

	va_start(ap, msg);
	log_msg_va(LOG_INFO, 0, msg, ap);
	va_end(ap);
}

void
log_notice(const char *msg, ...)
{
	va_list ap;

	va_start(ap, msg);
	log_msg_va(LOG_NOTICE, 0, msg, ap);
	va_end(ap);
}

void
log_warn(const char *msg, ...)
{
	va_list ap;

	va_start(ap, msg);
	log_msg_va(LOG_WARNING, 0, msg, ap);
	va_end(ap);
}

void
log_error(const char *msg, ...)
{
	va_list ap;

	va_start(ap, msg);
	log_msg_va(LOG_ERR, 0, msg, ap);
	va_end(ap);
}

void
log_socket_error(const char *msg, ...)
{
	va_list ap;

	va_start(ap, msg);
	log_msg_va(LOG_ERR, 1, msg, ap);
	va_end(ap);
}

void
log_fatal(const char *msg, ...)
{
	va_list ap;

	va_start(ap, msg);
	log_msg_va(LOG_CRIT, 0, msg, ap);
	va_end(ap);
}

void
log_msg_va(int lvl, int serr, const char *msg, va_list ap)
{
	if (debug) {
		if (lvl >= min_log_level) {
			vfprintf(log_file, msg, ap);
			if (serr)
				fprintf(log_file, ": %s",
				    socket_error_string(-1));
			fputs("\n", log_file);
			fflush(log_file);
			if (lvl <= LOG_CRIT)
				abort();
		}
	} else
		vsyslog(lvl, msg, ap);
}

void
log_set_min_level(int lvl)
{
	min_log_level = lvl;
}

int
log_get_min_level(void)
{
	return min_log_level;
}

void
log_set_file(FILE *fp)
{
	if (!fp)
		log_file = stderr;
	else
		log_file = fp;
}

void
log_set_scrub(int scrub)
{
	log_do_scrub = scrub;
}

int
log_get_scrub(void)
{
	return log_do_scrub;
}

const char *
log_scrub(const char *what)
{
	if (log_do_scrub)
		return "(scrubbed)";

	return what;
}
