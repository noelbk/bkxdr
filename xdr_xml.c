/*
 *  xdr_xml.c - make Sun's XDR routines speak XML instead if binary XDR
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

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "rpc/xdr.h"
#include "base64.h"

#define XML_STATIC 
#include "expat.h"


/*
<a>
  <b>
    <c type="something">
      <d type="int">123</d>
      <e type="int">456</e>
      <f type="string">abcdef</f>
      <array type="int[]">
        <elt type="int" index="0">1</elt>
        <elt type="int" index="1">2</elt>
        <elt type="int" index="2">3</elt>
      </array>
      <list type="j">
	<val type="int">456</val>
        <next type="j">
	  <val type="int">123</val>
	  <next type="j"></next>
	</next>
      </list>
    </c>
  </b>
</a>
*/

#ifndef ASSERTB
#define ASSERTB(cond) if( !(cond) ) { break; }
#endif

#ifndef BUF_ADD
#define BUF_ADD(buf, len, off) \
    ASSERTB((off)>=0);	       \
    (buf) += (off);	       \
    (len) -= (off);
#endif // BUF_ADD

typedef struct xdrxml_node_t {
    struct xdrxml_node_t *parent, *child, *last_child, *next;
    char *name;
    char *value;
    int  len;
} xdrxml_node_t;

typedef struct xdrxml_ctx_t {
    xdrxml_node_t *root;    /* the root of the whole tree */
    xdrxml_node_t *current; /* the current node I'm filling (below the root) */
    int depth;

    /* xdrxml_dump: the output xdr stream */
    XDR *xdr_out;           

    /* xdrxml_stream: my parser, XML nodes, and callback */
    XDR *xdr_self;          
    XML_Parser parser;
    xdrxml_stream_recv_func_t func;
    void *farg;
    int parsed_len;

    char errbuf[4096];
} xdrxml_ctx_t;

void
xdrxml_node_free(xdrxml_node_t *root) {
    xdrxml_node_t *child, *next;
    for(child=root->child; child; child=next) {
	next = child->next;
	xdrxml_node_free(child);
    }
    if( root->name ) free(root->name);
    if( root->name ) free(root->value);
    free(root);
}

void
xdrxml_node_print(xdrxml_node_t *root, int depth) {
    xdrxml_node_t *child, *next;
    int i;

    for(i=0; i<depth; i++) {
	printf(" ");
    }
    printf("%s: %s\n", root->name, root->value);

    for(child=root->child; child; child=next) {
	next = child->next;
	xdrxml_node_print(child, depth+1);
    }
}

xdrxml_node_t*
xdrxml_node_new_child(xdrxml_node_t *parent, const char *name) {
    xdrxml_node_t *child;
    
    child = calloc(sizeof(*child), 1);
    child->parent = parent;
    child->name = strdup(name);
    
    if( parent ) {
	if( parent->last_child ) {
	    child->next = parent->last_child->next;
	    parent->last_child->next = child;
	}
	else {
	    parent->child = child;
	}
	parent->last_child = child;
    }
    return child;
}

void
xdrxml_start_element(void *userData, const char *name, const char **atts) {
    xdrxml_ctx_t *ctx = (xdrxml_ctx_t*)userData;
    xdrxml_node_t *current = ctx->current;
    xdrxml_node_t *child;
    
    child = xdrxml_node_new_child(current, name);
    if( !ctx->root ) {
	ctx->root = child;
    }
    ctx->current = child;
    ctx->depth++;
}

void
xdrxml_end_element(void *userData, const char *name) {
    xdrxml_ctx_t *ctx = (xdrxml_ctx_t*)userData;
    xdrxml_node_t *current = ctx->current;
    char *p;

    /* trim trailing spaces */
    for(p = current->value + current->len-1;
	current->len>0 && isspace(*p); current->len--, p--) {
	*p = 0;
    }

    ctx->current = current->parent;
    ctx->depth--;

    
    /* if I've got a complete top-level element, call my callback and
       free it */
    if( ctx->func && ctx->depth==1 ) {
	ctx->current = ctx->current->child;
	ctx->func(ctx->xdr_self, ctx->farg);
	ctx->current = ctx->root;
	xdrxml_node_free(ctx->current->child);
	ctx->current->child = 0;
	ctx->current->last_child = 0;
    }
}

void
xdrxml_char_data(void *userData, const XML_Char *value, int len) {
    xdrxml_ctx_t *ctx = (xdrxml_ctx_t*)userData;
    xdrxml_node_t *current = ctx->current;
    int i;
    
    /* ignore spaces in root */
    if( !current->parent ) {
	return;
    }

    /* trim leading whitespace */
    if( current->len <= 0 ) {
	for(; len>0 && isspace(*value); value++, len--);
    }

    if( len > 0 ) {
	i = current->len;
	current->value = realloc(current->value, i+len+1);
	strncpy(current->value+i, value, len);
	current->value[i+len] = 0;
	current->len += len;
    }
}

int
xdrxml_printf(XDR *xdr, char *fmt, ...) {
    xdrxml_ctx_t  *ctx = (xdrxml_ctx_t*)xdr->x_private;
    XDR *xdr_out = ctx->xdr_out;
    char buf[4096];
    int n;
    va_list va;

    va_start(va, fmt);
    n = vsnprintf(buf, sizeof(buf), fmt, va);
    xdr_out->x_ops->x_putbytes(xdr_out, buf, n);
    va_end(va);
    return n;
}

bool_t
xdrxml_type(XDR *xdr, enum xdr_type type, void *p, 
	     xdrproc_t eltfunc, int *nelts, int eltsize, int maxelts) {
    xdrxml_ctx_t  *ctx = (xdrxml_ctx_t*)xdr->x_private;
    xdrxml_node_t *current = ctx->current;
    char *strval = current && current->value ? current->value : "";
    char *c;
    int i;

    switch(type) { 
    case XDR_TYPE_BYTES: {
	if( xdr->x_op == XDR_ENCODE ) {
	    c = *(char**)p;
	    if( c && *nelts > 0 ) {
		char *outp;
		i = *nelts * 2 + 4 + 1;
		outp = malloc(i);
		i = base64_enc(c, *nelts, outp, i);
		if( i > 0 ) {
		    xdrxml_printf(xdr, "%s", outp);
		}
		free(outp);
	    }
	}
	else if( xdr->x_op == XDR_DECODE ) {
	    c = (char*)malloc(current->len);
	    i = base64_dec(strval, current->len, c, current->len);
	    *(char**)p = c;
	    *nelts = i;
	}
	break;
    }

    case XDR_TYPE_BOOL:
	if( xdr->x_op == XDR_ENCODE ) {
	    xdrxml_printf(xdr, "%s", *(bool_t*)p ? "1" : "0");
	}
	else if( xdr->x_op == XDR_DECODE ) {
	    *(bool_t*)p = strtoul(strval, &c, 0);
	}
	break;

    case XDR_TYPE_ENUM:
	if( xdr->x_op == XDR_ENCODE ) {
	    xdrxml_printf(xdr, "%lu", (unsigned long)*(enum_t*)p);
	}
	else if( xdr->x_op == XDR_DECODE ) {
	    *(enum_t*)p = strtoul(strval, &c, 0);
	}
	break;

    case XDR_TYPE_CHAR:
	if( xdr->x_op == XDR_ENCODE ) {
	    xdrxml_printf(xdr, "%ld", (long)*(char*)p);
	}
	else if( xdr->x_op == XDR_DECODE ) {
	    *(char*)p = (char)strtol(strval, &c, 0);
	}
	break;

    case XDR_TYPE_SHORT:
	if( xdr->x_op == XDR_ENCODE ) {
	    xdrxml_printf(xdr, "%ld", (long)*(short*)p);
	}
	else if( xdr->x_op == XDR_DECODE ) {
	    *(short*)p = (short)strtol(strval, &c, 0);
	}
	break;

    case XDR_TYPE_INT:
	if( xdr->x_op == XDR_ENCODE ) {
	    xdrxml_printf(xdr, "%ld", (long)*(int*)p);
	}
	else if( xdr->x_op == XDR_DECODE ) {
	    *(int*)p = strtol(strval, &c, 0);
	}
	break;

    case XDR_TYPE_LONG:
	if( xdr->x_op == XDR_ENCODE ) {
	    xdrxml_printf(xdr, "%ld", (long)*(long*)p);
	}
	else if( xdr->x_op == XDR_DECODE ) {
	    *(long*)p = strtol(strval, &c, 0);
	}
	break;

    case XDR_TYPE_INT8:
	if( xdr->x_op == XDR_ENCODE ) {
	    xdrxml_printf(xdr, "%ld", (long)*(int8_t*)p);
	}
	else if( xdr->x_op == XDR_DECODE ) {
	    *(int8_t*)p = (int8_t)strtol(strval, &c, 0);
	}
	break;

    case XDR_TYPE_INT16:
	if( xdr->x_op == XDR_ENCODE ) {
	    xdrxml_printf(xdr, "%ld", (long)*(int16_t*)p);
	}
	else if( xdr->x_op == XDR_DECODE ) {
	    *(int16_t*)p = (int16_t)strtol(strval, &c, 0);
	}
	break;

    case XDR_TYPE_INT32:
	if( xdr->x_op == XDR_ENCODE ) {
	    xdrxml_printf(xdr, "%ld", (long)*(int32_t*)p);
	}
	else if( xdr->x_op == XDR_DECODE ) {
	    *(int32_t*)p = strtol(strval, &c, 0);
	}
	break;

    case XDR_TYPE_UCHAR:
	if( xdr->x_op == XDR_ENCODE ) {
	    xdrxml_printf(xdr, "%ld", (long)*(int32_t*)p);
	}
	else if( xdr->x_op == XDR_DECODE ) {
	    *(int32_t*)p = strtoul(strval, &c, 0);
	}
	break;
	
    case XDR_TYPE_USHORT:
	if( xdr->x_op == XDR_ENCODE ) {
	    xdrxml_printf(xdr, "%ld", (unsigned long)*(unsigned short*)p);
	}
	else if( xdr->x_op == XDR_DECODE ) {
	    *(unsigned short*)p = (unsigned short)strtoul(strval, &c, 0);
	}
	break;

    case XDR_TYPE_UINT:
	if( xdr->x_op == XDR_ENCODE ) {
	    xdrxml_printf(xdr, "%ld", (unsigned long)*(unsigned int*)p);
	}
	else if( xdr->x_op == XDR_DECODE ) {
	    *(unsigned int*)p = strtoul(strval, &c, 0);
	}
	break;

    case XDR_TYPE_ULONG:
	if( xdr->x_op == XDR_ENCODE ) {
	    xdrxml_printf(xdr, "%ld", (unsigned long)*(unsigned long*)p);
	}
	else if( xdr->x_op == XDR_DECODE ) {
	    *(unsigned long*)p = strtoul(strval, &c, 0);
	}
	break;

    case XDR_TYPE_UINT8:
	if( xdr->x_op == XDR_ENCODE ) {
	    xdrxml_printf(xdr, "%lu", (unsigned long)*(uint8_t*)p);
	}
	else if( xdr->x_op == XDR_DECODE ) {
	    *(uint8_t*)p = (uint8_t)strtoul(strval, &c, 0);
	}
	break;
	
    case XDR_TYPE_UINT16:
	if( xdr->x_op == XDR_ENCODE ) {
	    xdrxml_printf(xdr, "%lu", (unsigned long)*(uint16_t*)p);
	}
	else if( xdr->x_op == XDR_DECODE ) {
	    *(uint16_t*)p = (uint16_t)strtoul(strval, &c, 0);
	}
	break;

    case XDR_TYPE_UINT32:
	if( xdr->x_op == XDR_ENCODE ) {
	    xdrxml_printf(xdr, "%lu", (unsigned long)*(uint32_t*)p);
	}
	else if( xdr->x_op == XDR_DECODE ) {
	    *(uint32_t*)p = strtoul(strval, &c, 0);
	}
	break;

    case XDR_TYPE_UHYPER:
    case XDR_TYPE_UINT64:
	if( xdr->x_op == XDR_ENCODE ) {
	    xdrxml_printf(xdr, "%lu%lu", 
			  (unsigned long)(*(uint64_t*)p >> 32), 
			  (unsigned long)(*(uint64_t*)p & 0xffffffff));
	}
	else if( xdr->x_op == XDR_DECODE ) {
	    /* todo - support 64 bits */
	    //*(uint64_t*)p = strtoll(strval, &c, 0);
	}
	break;

    case XDR_TYPE_FLOAT:
	if( xdr->x_op == XDR_ENCODE ) {
	    xdrxml_printf(xdr, "%f", *(float*)p);
	}
	else if( xdr->x_op == XDR_DECODE ) {
	    *(float*)p = (float)strtod(strval, &c);
	}
	break;

    case XDR_TYPE_DOUBLE:
	if( xdr->x_op == XDR_ENCODE ) {
	    xdrxml_printf(xdr, "%lf", *(double*)p);
	}
	else if( xdr->x_op == XDR_DECODE ) {
	    *(double*)p = strtod(strval, &c);
	}
	break;

    case XDR_TYPE_STRING:
	if( xdr->x_op == XDR_ENCODE ) {
	    xdrxml_printf(xdr, "%s", *(char**)p);
	}
	else if( xdr->x_op == XDR_DECODE ) {
	    *(char**)p = strdup(strval);
	}
	break;

    case XDR_TYPE_ARRAY: {
	int i, e;

	c = *(char**)p;

	if( xdr->x_op == XDR_ENCODE ) {
	    xdr->x_label->depth++;
	    for(e=0; e<*nelts; e++) {
		xdrxml_printf(xdr, "\n");
		for(i=0; i<xdr->x_label->depth; i++) {
		    xdrxml_printf(xdr, "  ");
		}
		xdrxml_printf(xdr, "<elt index=\"%d\">", e);

		eltfunc(xdr, c);
		c += eltsize;

		if( 0 ) {
		    xdrxml_printf(xdr, "\n");
		    for(i=0; i<xdr->x_label->depth; i++) {
			xdrxml_printf(xdr, "  ");
		    }
		}
		xdrxml_printf(xdr, "</elt>");
	    }
	    xdr->x_label->depth--;
	}
	else if( xdr->x_op == XDR_DECODE ) {
	    xdrxml_node_t *current = ctx->current;
	    xdrxml_node_t *child;

	    /* count children */
	    for(child=current->child, i=0; child; child=child->next, i++);
	    *nelts = i;
	    if( *nelts == 0 ) {
		*(char**)p = 0;
		break;
	    }
	    c = malloc(*nelts * eltsize);
	    *(char**)p = c;
	    for(child=current->child, i=0; child; child=child->next, i+=eltsize) {
		ctx->current = child;
		eltfunc(xdr, c+i);
	    }
	    ctx->current = current;
	}
	
	break;
    }

    case XDR_TYPE_POINTER: {
	if( xdr->x_op == XDR_ENCODE ) {
	    char *c = *(char**)p;
	    if( c ) {
		return eltfunc(xdr, c);
	    }
	}
	else if( xdr->x_op == XDR_DECODE ) {
	    if( ctx->current->len || ctx->current->child ) {
		c = malloc(eltsize);
		*(char**)p = c;
		eltfunc(xdr, c);
	    }
	    else {
		*(char**)p = 0;
	    }
	}
	break;
    }

    default:
	return 0;
    }
    return 1;
}


void
xdrxml_label(XDR *xdr, int push) {
    xdrxml_ctx_t  *ctx = (xdrxml_ctx_t*)xdr->x_private;
    int i;

    if( xdr->x_op == XDR_ENCODE ) {
	if( push ) {
	    xdrxml_printf(xdr, "\n");
	    for(i=0; i<xdr->x_label->depth; i++) {
		xdrxml_printf(xdr, "  ");
	    }
	    xdrxml_printf(xdr, "<%s type=\"%s\">", xdr->x_label->name, xdr->x_label->type);
	    if( xdr->x_label->next ) {
		xdr->x_label->next->is_struct = 1;
	    }
	}
	else {
	    // if this closes a struct (nested labels), the closing bracket
	    // goes on a new line.  scalar => no spaces
	    if( xdr->x_label->is_struct ) {
		xdrxml_printf(xdr, "\n");
		for(i=0; i<xdr->x_label->depth; i++) {
		    xdrxml_printf(xdr, "  ");
		}
	    }
	    xdrxml_printf(xdr, "</%s>", xdr->x_label->name);
	}
    }
    else if( xdr->x_op == XDR_DECODE ) {
	if( push ) {
	    xdrxml_node_t *current = ctx->current;
	    xdrxml_node_t *child;
	    
	    if( !current ) {
		ctx->current = ctx->root;
		current = ctx->current;
	    }

	    if( !current ) {
		return;
	    }

	    /* find label as a child of the current node */
	    for(child = current->child; 
		child && strcmp(child->name, xdr->x_label->name)!=0; 
		child = child->next);

	    if( !child ) {
		/* no node found, append an empty one */
		child = xdrxml_node_new_child(current, xdr->x_label->name);
	    }
	    ctx->current = child;
	}
	else {
	    ctx->current  = ctx->current->parent;
	}
    }
}

static bool_t
xdrxml_getint32 (XDR *xdr, int32_t *lp)
{
    xdrxml_ctx_t *ctx = (xdrxml_ctx_t *)xdr->x_private;
    XDR *xdr_out = ctx->xdr_out;
    return xdr_out->x_ops->x_getint32(xdr, lp);
}

static bool_t
xdrxml_putint32 (XDR *xdr, int32_t *lp)
{
    xdrxml_ctx_t *ctx = (xdrxml_ctx_t *)xdr->x_private;
    XDR *xdr_out = ctx->xdr_out;
    return xdr_out->x_ops->x_putint32(xdr, lp);
}

static bool_t
xdrxml_getlong (XDR *xdr, long *lp)
{
    xdrxml_ctx_t *ctx = (xdrxml_ctx_t *)xdr->x_private;
    XDR *xdr_out = ctx->xdr_out;
    return xdr_out->x_ops->x_getlong(xdr, lp);
}

static bool_t
xdrxml_putlong (XDR *xdr, long *lp)
{
    xdrxml_ctx_t *ctx = (xdrxml_ctx_t *)xdr->x_private;
    XDR *xdr_out = ctx->xdr_out;
    return xdr_out->x_ops->x_putlong(xdr, lp);
}

static bool_t
xdrxml_getbytes (XDR *xdr, caddr_t addr, u_int len)
{
    xdrxml_ctx_t *ctx = (xdrxml_ctx_t *)xdr->x_private;
    XDR *xdr_out = ctx->xdr_out;
    return xdr_out->x_ops->x_getbytes(xdr, addr, len);
}

static bool_t
xdrxml_putbytes (XDR *xdr, char *addr, u_int len)
{
    xdrxml_ctx_t *ctx = (xdrxml_ctx_t *)xdr->x_private;
    XDR *xdr_out = ctx->xdr_out;
    return xdr_out->x_ops->x_putbytes(xdr, addr, len);
}

static u_int
xdrxml_getpos (XDR *xdr)
{
    xdrxml_ctx_t *ctx = (xdrxml_ctx_t *)xdr->x_private;
    if( xdr->x_op == XDR_ENCODE ) {
	XDR *xdr_out = ctx->xdr_out;
	return xdr_out ? xdr_out->x_ops->x_getpostn(xdr_out) : 0;
    }

    return ctx->parsed_len;
}

static bool_t
xdrxml_setpos (XDR *xdr, u_int pos)
{
    xdrxml_ctx_t *ctx = (xdrxml_ctx_t *)xdr->x_private;
    XDR *xdr_out = ctx->xdr_out;
    return xdr_out->x_ops->x_setpostn(xdr, pos);
}

static int32_t *
xdrxml_inline (XDR *xdr, u_int len)
{
    return NULL;
}

/* destroy an xdrxml stream */
void
xdrxml_destroy(XDR *xdr)
{
    xdrxml_ctx_t *ctx = (xdrxml_ctx_t *)xdr->x_private;
    XDR *xdr_out = ctx->xdr_out;

    if( xdr_out && xdr_out->x_ops && xdr_out->x_ops->x_destroy ) {
	xdr_out->x_ops->x_destroy(xdr_out);
    }
    if( ctx->root ) {
	xdrxml_node_free(ctx->root);
    }
    if( ctx->parser ) {
	XML_ParserFree(ctx->parser);
    }
    free(ctx);
}

static struct xdr_ops xdrxml_ops =
{
  xdrxml_getlong,		/* deserialize a long int */
  xdrxml_putlong,		/* serialize a long int */
  xdrxml_getbytes,		/* deserialize counted bytes */
  xdrxml_putbytes,		/* serialize counted bytes */
  xdrxml_getpos,		/* get offset in the stream */
  xdrxml_setpos,		/* set offset in the stream */
  xdrxml_inline,		/* prime stream for inline macros */
  xdrxml_destroy,		/* destroy stream */
  xdrxml_getint32,		/* deserialize a int */
  xdrxml_putint32		/* serialize a int */

  ,xdrxml_label
  ,xdrxml_type
};

int
xdrxml_fmt(xdrproc_t func, void *obj, char *buf, int len, int omit_root) {
    XDR xdr_out, xdr_xml, *xdr_init=0;
    int i, err=-1;
    char *orig = buf;

    do {
	if( !omit_root ) {
	    i = snprintf(buf, len, 
			 "<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>\n"
			 "<root>");
	    BUF_ADD(buf, len, i);
	}
	xdrmem_create(&xdr_out, buf, len, XDR_ENCODE);
	xdr_init = &xdr_out;
	xdrxml_create_xdr_write(&xdr_xml, &xdr_out);
	xdr_init = &xdr_xml;
	i = func(&xdr_xml, obj);
	if( i <= 0 ) break;
	i = xdr_getpos(&xdr_out);
	BUF_ADD(buf, len, i);

	if( !omit_root ) {
	    i = snprintf(buf, len, "\n</root>\n");
	    BUF_ADD(buf, len, i);
	}
	err= 0;
    } while(0);
    if( xdr_init ) {
	xdr_destroy(xdr_init);
    }
    return err ? err : buf-orig;
}

int
xdrxml_parse(xdrproc_t func, void *obj, char *buf, int len) {
    XDR xdr_xml, *xdr_init=0;
    xdrxml_ctx_t *ctx;
    int i = -1;

    do {
	ASSERTB(len>0);
	i = xdrxml_create_buf_read(&xdr_xml, buf, len);
	ASSERTB(i>=0);
	xdr_init = &xdr_xml;
	i = func(&xdr_xml, obj);
	ASSERTB(i>0);
	ctx = (xdrxml_ctx_t *)(xdr_xml.x_private);
	ASSERTB(ctx);
	i = ctx->parsed_len;
    } while(0);
    if( xdr_init ) {
	xdr_destroy(xdr_init);
    }
    return i;
}

/* create an XML output stream */
int
xdrxml_create_xdr_write(XDR *xdr, XDR *xdr_out) {
    int err=-1;
    xdrxml_ctx_t *ctx;

    do {
	xdr_init(xdr);
	xdr->x_op = xdr_out->x_op;
	xdr->x_ops = (struct xdr_ops*)&xdrxml_ops;

	ctx = (xdrxml_ctx_t*)calloc(1, sizeof(*ctx));
	ctx->xdr_self = xdr;
	ctx->xdr_out = xdr_out;
	xdr->x_private = (caddr_t)ctx;

	xdr->x_handy = 0;
	xdr->x_base = 0;

    } while(0);
    return err;
}

/* create an XML output stream */
int
xdrxml_create_buf_read(XDR *xdr, char *buf, int len) {
    xdrxml_ctx_t *ctx;
    int i, err=-1;

    do {
	i = xdrxml_stream_open(xdr, 0, 0);
	ASSERTB(i>=0);
	xdrxml_stream_recv(xdr, buf, len, 1);
	ctx = (xdrxml_ctx_t *)(xdr->x_private);
	ASSERTB(!*ctx->errbuf);
	ASSERTB(ctx->parsed_len > 0);
	err = 0;
    } while(0);
    return err;
}

int
xdrxml_stream_open(XDR *xdr, xdrxml_stream_recv_func_t func, void *farg) {
    xdrxml_ctx_t *ctx=0;

    do {
	ctx = (xdrxml_ctx_t*)calloc(1, sizeof(*ctx));
	ASSERTB(ctx);
	ctx->xdr_self = xdr;
	ctx->func = func;
	ctx->farg = farg;

	ctx->parser = XML_ParserCreate(NULL);
	ASSERTB(ctx->parser);
	XML_SetUserData(ctx->parser, ctx);
	XML_SetElementHandler(ctx->parser, xdrxml_start_element, xdrxml_end_element);
	XML_SetCharacterDataHandler(ctx->parser, xdrxml_char_data);
    
	xdr_init(xdr);
	xdr->x_op = XDR_DECODE;
	xdr->x_ops = (struct xdr_ops*)&xdrxml_ops;
	xdr->x_private = (caddr_t)ctx;
	xdr->x_handy = 0;
	xdr->x_base = 0;
    } while(0);

    return 0;
}

void
xdrxml_stream_recv(XDR *xdr, char *buf, int len, int final) {
    xdrxml_ctx_t *ctx = (xdrxml_ctx_t *)(xdr->x_private);
    int i;
    do {
	snprintf(ctx->errbuf, sizeof(ctx->errbuf), 
		 "XML_Parse");
	i = XML_Parse(ctx->parser, buf, len, final);
	ASSERTB(i>0);
	ctx->parsed_len += len;
	if( i == XML_STATUS_ERROR ) {
	    snprintf(ctx->errbuf, sizeof(ctx->errbuf),
		     "%s at line %d\n",
		     XML_ErrorString(XML_GetErrorCode(ctx->parser)),
		     XML_GetCurrentLineNumber(ctx->parser));
	}
	else {
	    *ctx->errbuf = 0;
	}
    } while(0);
}

void
xdrxml_stream_close(XDR *xdr) {
    xdrxml_destroy(xdr);
}
