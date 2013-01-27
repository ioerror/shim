#ifndef _HTTPCONN_H_
#define _HTTPCONN_H_

enum http_version {
	HTTP_UNKNOWN,
	HTTP_10,
	HTTP_11
};

enum http_state {
	HTTP_STATE_IDLE,
	HTTP_STATE_CONNECTING,
	HTTP_STATE_READ_FIRSTLINE,
	HTTP_STATE_READ_HEADERS,
	HTTP_STATE_READ_BODY,
	HTTP_STATE_MANGLED,
	HTTP_STATE_TUNNEL_CONNECTING,
	HTTP_STATE_TUNNEL_OPEN,
	HTTP_STATE_TUNNEL_FLUSHING
};

enum http_type {
	HTTP_CLIENT,
	HTTP_SERVER
};

enum http_method {
	METH_GET,
	METH_HEAD,
	METH_POST,
	METH_PUT,
	METH_CONNECT
};

enum http_te {
	TE_IDENTITY,
	TE_CHUNKED
};

enum http_conn_error {
	ERROR_NONE,
	ERROR_CONNECT_FAILED,
	ERROR_IDLE_CONN_TIMEDOUT,
	ERROR_CLIENT_EXPECTATION_FAILED,
	ERROR_CLIENT_POST_WITHOUT_LENGTH,
	ERROR_INCOMPLETE_HEADERS,
	ERROR_INCOMPLETE_BODY,
	ERROR_HEADER_PARSE_FAILED,
	ERROR_CHUNK_PARSE_FAILED,
	ERROR_WRITE_FAILED,
	ERROR_TUNNEL_CONNECT_FAILED,
	ERROR_TUNNEL_CLOSED
};

#define HTTP_ERROR_RESPONSE(c) (c >= 400 && c <= 599)

struct evbuffer;
struct event_base;
struct evdns_base;
struct http_conn;
struct header_list;
struct url;

struct http_request {
	TAILQ_ENTRY(http_request) next;
	enum http_method meth;
	struct url *url;
	enum http_version vers;
	struct header_list *headers;
};
TAILQ_HEAD(http_request_list, http_request);

struct http_response {
	enum http_version vers;
	int code;
	char *reason;
	struct header_list *headers;
};

struct http_cbs {
	void (*on_connect)(struct http_conn *, void *);
	void (*on_error)(struct http_conn *, enum http_conn_error, void *);
	void (*on_client_request)(struct http_conn *, struct http_request *, void *);
	void (*on_server_continuation)(struct http_conn *, void *);
	void (*on_server_response)(struct http_conn *, struct http_response *, void *);
	void (*on_read_body)(struct http_conn *, struct evbuffer *, void *);
	void (*on_msg_complete)(struct http_conn *, void *);

	/* called when it is ok to write more data after choaking */
	void (*on_write_more)(struct http_conn *, void *);

	void (*on_flush)(struct http_conn *, void *);
};

struct http_conn *http_conn_new(struct event_base *base, evutil_socket_t sock,
				enum http_type type, const struct http_cbs *cbs,
				void *cbarg);

int http_conn_connect(struct http_conn *conn, struct evdns_base *dns,
		      int family, const char *host, int port);

void http_conn_free(struct http_conn *conn);

void http_conn_write_request(struct http_conn *conn, struct http_request *req);
int http_conn_expect_continue(struct http_conn *conn);
void http_conn_write_continue(struct http_conn *conn);
void http_conn_write_response(struct http_conn *conn, struct http_response *resp);

/* return: 0 on choaked, 1 on queued. */
int http_conn_write_buf(struct http_conn *conn, struct evbuffer *buf);
void http_conn_write_finished(struct http_conn *conn);

int http_conn_current_message_has_body(struct http_conn *conn);
void http_conn_set_current_message_bodyless(struct http_conn *conn);

enum http_te http_conn_get_current_message_body_encoding(struct http_conn *conn);
ev_int64_t http_conn_get_current_message_body_length(struct http_conn *conn);
void http_conn_set_output_encoding(struct http_conn *conn, enum http_te te);
int http_conn_is_persistent(struct http_conn *conn);
void http_conn_disable_persistence(struct http_conn *conn);

/* turn read off/on; useful for when the other end is choking */
void http_conn_stop_reading(struct http_conn *conn);
void http_conn_start_reading(struct http_conn *conn);

void http_conn_flush(struct http_conn *conn);

void http_conn_send_error(struct http_conn *conn, int code,
			  const char *fmt, ...);

int http_conn_start_tunnel(struct http_conn *conn, struct evdns_base *dns,
		      int family, const char *host, int port);

const char *http_conn_error_to_string(enum http_conn_error err);
const char *http_method_to_string(enum http_method m);
const char *http_version_to_string(enum http_version v);

void http_request_free(struct http_request *req);
void http_response_free(struct http_response *req);

#endif
