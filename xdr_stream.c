/*
 *  xdr_stream.c - consume streams of structs in XML
 *  Noel Burton-Krahn <noel@burton-krahn.com>
 *  Jan 1, 2003
 *
 *  Copyright (C) Noel Burton-Krahn, 2004, and is released under the
 *  GPL version 2 (see below).
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program (see the file COPYING included with this
 *  distribution); if not, write to the Free Software Foundation, Inc.,
 *  59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "xdr_stream.h"
#include "debug.h"
#include "sock.h"

// kludge - should be dynamically sized
#define XDR_STREAM_BUFLEN 32767

struct xdr_stream_s {
    void *arg;
    xdr_stream_read_t read;
    xdr_stream_write_t write;
    xdr_stream_close_t close;
};

xdr_stream_t*
xdr_stream_new(xdr_stream_read_t read,
	       xdr_stream_write_t write,
	       xdr_stream_close_t close,
	       void *arg
	       ) {
    xdr_stream_t *stream=0;
    int err=-1;
    do {
	stream = calloc(1, sizeof(*stream));
	assertb(stream);
	stream->arg   = arg;
	stream->read = read;
	stream->write = write;
	stream->close = close;
	err = 0;
    } while(0);
    if( err ) {
	if(stream) {
	    xdr_stream_delete(stream);
	    stream = 0;
	}
    }
    return stream;
}

int
xdr_stream_delete(xdr_stream_t *stream) {
    if( stream ) {
	if( stream->close ) {
	    stream->close(stream->arg);
	}
	free(stream);
    }
    return 0;
}

static
int
xdr_stream_sock_read(void *arg, char *buf, int len) {
    return sock_recv_timeout((sock_t)arg, buf, len, 1000);
}

static
int
xdr_stream_sock_write(void *arg, char *buf, int len) {
    return sock_send_timeout((sock_t)arg, buf, len, 1000);
}

static
int
xdr_stream_sock_close(void *arg) {
    return sock_close((sock_t)arg);
}

xdr_stream_t*
xdr_stream_new_sock(sock_t sock, int close) {
    return xdr_stream_new(xdr_stream_sock_read, 
			  xdr_stream_sock_write, 
			  close ? xdr_stream_sock_close : 0, 
			  (void*)sock);
}

void*
xdr_stream_arg(xdr_stream_t *stream) {
    return stream->arg;
}


int
xdr_stream_send(xdr_stream_t *stream, xdrproc_t func, void *msg) {
    int i, n, err=-1;
    XDR xdr;
    char buf[XDR_STREAM_BUFLEN];

    do {
	xdrmem_create(&xdr, buf+2, sizeof(buf)-2, XDR_ENCODE);
	i = func(&xdr, msg);
	n = xdr_getpos(&xdr);
	xdr_destroy(&xdr);
	assertb(i);
	assertb(n>0 && n<sizeof(buf));
	*(unsigned short*)buf = htons((unsigned short)n);
	i = stream->write(stream->arg, buf, n+2);
	assertb_sockerr(i==n+2);
	err = 0;
    } while(0);
    return err ? err : n;
}

int
xdr_stream_recv(xdr_stream_t *stream, xdrproc_t func, void *msg) {
    int i, n=0, err=-1;
    XDR xdr;
    char buf[XDR_STREAM_BUFLEN];

    do {
	i = stream->read(stream->arg, buf, 2);
	if( i == 0 ) {
	    err = 0;
	    n = 0;
	    break;
	}
	assertb_sockerr(i==2);
	n = ntohs(*(unsigned short*)buf);
	assertb(n>0 && n<sizeof(buf));	    
	i = stream->read(stream->arg, buf, n);
	assertb_sockerr(i==n);
	xdrmem_create(&xdr, buf, n, XDR_DECODE);
	i = func(&xdr, msg);
	xdr_destroy(&xdr);
	assertb(i);
	err = 0;
    } while(0);
    return err ? err : n;
}

int
xdr_copy(void *dst, void *src, xdrproc_t func) {
    int i, n, m, err=-1;
    XDR xdr;
    char buf[XDR_STREAM_BUFLEN];

    do {
	xdrmem_create(&xdr, buf, sizeof(buf), XDR_ENCODE);
	i = func(&xdr, src);
	n = xdr_getpos(&xdr);
	xdr_destroy(&xdr);
	assertb(i);
	assertb(n>0 && n<sizeof(buf));

	xdrmem_create(&xdr, buf, n, XDR_DECODE);
	i = func(&xdr, dst);
	m = xdr_getpos(&xdr);
	xdr_destroy(&xdr);
	assertb(i);
	assertb(m==n);

	err = 0;
    } while(0);
    return err ? err : n;
}
