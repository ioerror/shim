#include "netheaders.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <syslog.h>
#include <stdarg.h>
/* XXX unistd/getopt isnt available everywhere! */
#include <unistd.h>
#ifndef WIN32
#include <signal.h>
#endif
#include <event2/event.h>
#include <event2/dns.h>
#include <event2/util.h>

#include "config.h"
#include "proxy.h"
#include "conn.h"
#include "log.h"
#include "util.h"

#define DEFAULT_LISTEN_ADDR "127.0.0.1"
#define DEFAULT_LISTEN_PORT "8123"

int debug;

static void
set_socks_server(const char *socks)
{
	struct url *url;
	enum socks_ver socksv;

	url = url_tokenize(socks);
	if (!url || !url->scheme) {
		log_error("shim: bad socks server, %s", socks);
		exit(1);
	}
	if (url->port < 0)
		url->port = 1080;

	if (!evutil_ascii_strcasecmp(url->scheme, "socks4"))
		socksv = SOCKS_4;
	else if (!evutil_ascii_strcasecmp(url->scheme, "socks4a"))
		socksv = SOCKS_4a;
	else {
		log_error("shim: unknown socks version, %s",
			  url->scheme);
		exit(1);
	}

	if (conn_set_socks_server(url->host, url->port, socksv) < 0)
		exit(1);

	url_free(url);	
}

static void
start_listening(struct event_base *base, struct evdns_base *dns,
		const char *laddr, const char *lport)
{
	struct evutil_addrinfo hints;
	struct evutil_addrinfo *ai = NULL;
	int ret;

	if (!evutil_ascii_strcasecmp(laddr, "any"))
		laddr = NULL;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	/* turn NULL hostname into INADDR_ANY, and skip looking up any address
	 * types we don't have an interface to connect to. */
	hints.ai_flags = EVUTIL_AI_PASSIVE|EVUTIL_AI_ADDRCONFIG;

	ret = evutil_getaddrinfo(laddr, lport, &hints, &ai);
	if (ret < 0) {
		log_error("shim: bad listen address: %s",
			  evutil_gai_strerror(ret));
		exit(1);
        }
	
	if (proxy_init(base, dns, ai->ai_addr, ai->ai_addrlen) < 0)
		exit(1);
}

static void
decrease_log_verbosity(void)
{
	switch (log_get_min_level()) {
	case LOG_DEBUG:
		log_set_min_level(LOG_INFO);
		break;
	case LOG_INFO:
		log_set_min_level(LOG_NOTICE);
		break;
	case LOG_NOTICE:
		log_set_min_level(LOG_WARNING);
		break;
	case LOG_WARNING:
		log_set_min_level(LOG_ERR);
		break;
	case LOG_ERR:
		log_set_min_level(LOG_CRIT);
		break;
	default:
		break;
	}
}

static void
increase_log_verbosity(void)
{
	switch (log_get_min_level()) {
	case LOG_CRIT:
		log_set_min_level(LOG_ERR);
		break;
	case LOG_ERR:
		log_set_min_level(LOG_WARNING);
		break;
	case LOG_WARNING:
		log_set_min_level(LOG_NOTICE);
		break;
	case LOG_NOTICE:
		log_set_min_level(LOG_INFO);
		break;
	case LOG_INFO:
		log_set_min_level(LOG_DEBUG);
		break;
	default:
		break;
	}
}

static void
init_socket_stuff()
{
#ifdef WIN32
	WSADATA WSAData;
	WSAStartup(0x101, &WSAData);
#else
	if (signal(SIGPIPE, SIG_IGN) == SIG_ERR) {
		perror("signal");
		exit(1);
	}
#endif
}

static void
usage(void)
{
	printf("shim [-l host] [-p port] [-dqVv] "
	       "[ socks_version://address[:port] ]\n");
	exit(1);
}

int
main(int argc, char **argv)
{
	struct event_base *base;
	struct evdns_base *dns = NULL;
	int opt;
	const char *laddr, *lport;

	log_set_file(NULL);

	laddr = DEFAULT_LISTEN_ADDR;
	lport = DEFAULT_LISTEN_PORT;

	while ((opt = getopt(argc, argv, "ldp:Vvq")) != -1) {
		switch (opt) {
		case 'd':
			debug = 1;
			break;
		case 'l':
			laddr = optarg;
			break;
		case 'p':
			lport = optarg;
			break;
		case 'V':
			printf("%s\n", PACKAGE_STRING);
			exit(1);
		case 'v':
			increase_log_verbosity();
			break;
		case 'q':
			decrease_log_verbosity();
			break;
		default:
			usage();
		}
	}

	argc -= optind;
	argv += optind;

	if (!debug && daemon(1, 0) == -1) {
		log_error("unable to daemonize");
		exit(1);
	}

	if (argc)
		set_socks_server(argv[0]);

	init_socket_stuff();

	base = event_base_new();
#ifndef DISABLE_DIRECT_CONNECTIONS
        dns = evdns_base_new(base, 1);
#endif
	start_listening(base, dns, laddr, lport);

	event_base_dispatch(base);

	return 0;	
}
