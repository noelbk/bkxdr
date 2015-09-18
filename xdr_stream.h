#ifndef XDR_STREAM_H_INCLUDED
#define XDR_STREAM_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include <rpc/xdr.h>
#include "sock.h"

struct xdr_stream_s;
typedef struct xdr_stream_s xdr_stream_t;

typedef int (*xdr_stream_read_t)(void *arg, char *buf, int len);
typedef int (*xdr_stream_write_t)(void *arg, char *buf, int len);
typedef int (*xdr_stream_close_t)(void *arg);

xdr_stream_t*
xdr_stream_new(xdr_stream_read_t read,
	       xdr_stream_write_t write,
	       xdr_stream_close_t close,
	       void *arg
	       );

xdr_stream_t*
xdr_stream_new_sock(sock_t sock, int close);

xdr_stream_t*
xdr_stream_new_buf(char *buf, int len);

void*
xdr_stream_arg(xdr_stream_t *stream);

int
xdr_stream_delete(xdr_stream_t *stream);

int
xdr_stream_send(xdr_stream_t *stream, xdrproc_t func, void *msg);

int
xdr_stream_recv(xdr_stream_t *stream, xdrproc_t func, void *msg);

int
xdr_copy(void *dst, void *src, xdrproc_t func);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // XDR_STREAM_H_INCLUDED
