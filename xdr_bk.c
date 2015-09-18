/*
 *  xdr_bk.c - handy routines for quick serilialization
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
#include "rpc/xdr.h"

#define assertb(cond) if( !(cond) ) { break; }

int
xdr_pack(xdrproc_t proc, void *x, 
	 enum xdr_op op, enum xdr_mode  mode, 
	 char *buf, int len) {
    int i, n, err=-1;
    XDR xdr, xdr_xml, *xdrp=&xdr;
    char *orig = buf;
    
    do {
	if( mode == XDR_XML ) {
	    if( op == XDR_DECODE ) {
		xdrxml_create_buf_read(&xdr_xml, buf, len);
	    }
	    else if( op == XDR_ENCODE ) {
		i = snprintf(buf, len,
			     "<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>\n"
			     "<root>");
		buf += i;
		len -= i;
		xdrmem_create(&xdr, buf, len, op);
		xdrxml_create_xdr_write(&xdr_xml, &xdr);
	    }
	    xdrp = &xdr_xml;
	}
	else {
	    xdrmem_create(&xdr, buf, len, op);
	    xdrp = &xdr;
	}

	i = proc(xdrp, x);
	assertb(i);
	n = xdr_getpos(xdrp);
	buf += n;
	len -= n;
	
	if( mode == XDR_XML ) {
	    if( op == XDR_ENCODE ) {
		i = snprintf(buf, len, "\n</root>\n");
		buf += i;
		len -= i;
	    }
	}



	xdr_destroy(xdrp);
	err = 0;
    } while(0);
    return err ? err : buf-orig;
}
