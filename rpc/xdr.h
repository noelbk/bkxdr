/*
 * Sun RPC is a product of Sun Microsystems, Inc. and is provided for
 * unrestricted use provided that this legend is included on all tape
 * media and as a part of the software program in whole or part.  Users
 * may copy or modify Sun RPC without charge, but are not authorized
 * to license or distribute it to anyone else except as part of a product or
 * program developed by the user.
 *
 * SUN RPC IS PROVIDED AS IS WITH NO WARRANTIES OF ANY KIND INCLUDING THE
 * WARRANTIES OF DESIGN, MERCHANTIBILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE, OR ARISING FROM A COURSE OF DEALING, USAGE OR TRADE PRACTICE.
 *
 * Sun RPC is provided with no support and without any obligation on the
 * part of Sun Microsystems, Inc. to assist in its use, correction,
 * modification or enhancement.
 *
 * SUN MICROSYSTEMS, INC. SHALL HAVE NO LIABILITY WITH RESPECT TO THE
 * INFRINGEMENT OF COPYRIGHTS, TRADE SECRETS OR ANY PATENTS BY SUN RPC
 * OR ANY PART THEREOF.
 *
 * In no event will Sun Microsystems, Inc. be liable for any lost revenue
 * or profits or other special, indirect and consequential damages, even if
 * Sun has been advised of the possibility of such damages.
 *
 * Sun Microsystems, Inc.
 * 2550 Garcia Avenue
 * Mountain View, California  94043
 */

/*
 * xdr.h, External Data Representation Serialization Routines.
 *
 * Copyright (C) 1984, Sun Microsystems, Inc.
 */

#ifndef _RPC_XDR_H
#define _RPC_XDR_H 1

//#include <features.h>
#include <sys/types.h>
#include <rpc/types.h>

/* We need FILE.  */
#include <stdio.h>



/*
 * XDR provides a conventional way for converting between C data
 * types and an external bit-string representation.  Library supplied
 * routines provide for the conversion on built-in C data types.  These
 * routines and utility routines defined here are used to help implement
 * a type encode/decode routine for each user-defined type.
 *
 * Each data type provides a single procedure which takes two arguments:
 *
 *      bool_t
 *      xdrproc(xdrs, argresp)
 *              XDR *xdrs;
 *              <type> *argresp;
 *
 * xdrs is an instance of a XDR handle, to which or from which the data
 * type is to be converted.  argresp is a pointer to the structure to be
 * converted.  The XDR handle contains an operation field which indicates
 * which of the operations (ENCODE, DECODE * or FREE) is to be performed.
 *
 * XDR_DECODE may allocate space if the pointer argresp is null.  This
 * data can be freed with the XDR_FREE operation.
 *
 * We write only one procedure per data type to make it easy
 * to keep the encode and decode procedures for a data type consistent.
 * In many cases the same code performs all operations on a user defined type,
 * because all the hard work is done in the component type routines.
 * decode as a series of calls on the nested data types.
 */

/*
 * Xdr operations.  XDR_ENCODE causes the type to be encoded into the
 * stream.  XDR_DECODE causes the type to be extracted from the stream.
 * XDR_FREE can be used to release the space allocated by an XDR_DECODE
 * request.
 */
enum xdr_op {
  XDR_ENCODE = 0,
  XDR_DECODE = 1,
  XDR_FREE = 2
};

enum xdr_mode {
    XDR_BINARY = 0,
    XDR_XML = 1
};

/*
 * This is the number of bytes per unit of external data.
 */
#define BYTES_PER_XDR_UNIT	(4)
/*
 * This only works if the above is a power of 2.  But it's defined to be
 * 4 by the appropriate RFCs.  So it will work.  And it's normally quicker
 * than the old routine.
 */
#if 1
#define RNDUP(x)  (((x) + BYTES_PER_XDR_UNIT - 1) & ~(BYTES_PER_XDR_UNIT - 1))
#else /* this is the old routine */
#define RNDUP(x)  ((((x) + BYTES_PER_XDR_UNIT - 1) / BYTES_PER_XDR_UNIT) \
		    * BYTES_PER_XDR_UNIT)
#endif

/* NBK: this is to keep a stack of structure elements for XML */
typedef struct XDR_LABEL XDR_LABEL;
struct XDR_LABEL {
    struct XDR_LABEL *next;
    char *name;
    char *type;
    int  depth;
    int  is_struct; /* true iff this contains sub-labels */
};

/* NBK - types for xdr_type() */
enum xdr_type {
    XDR_TYPE_NULL=0

    /* xdr.c */
    ,XDR_TYPE_VOID
    ,XDR_TYPE_INT
    ,XDR_TYPE_UINT
    ,XDR_TYPE_LONG
    ,XDR_TYPE_ULONG
    ,XDR_TYPE_HYPER
    ,XDR_TYPE_UHYPER
    ,XDR_TYPE_SHORT
    ,XDR_TYPE_USHORT
    ,XDR_TYPE_CHAR
    ,XDR_TYPE_UCHAR
    ,XDR_TYPE_BOOL
    ,XDR_TYPE_ENUM
    ,XDR_TYPE_OPAQUE
    ,XDR_TYPE_BYTES
    ,XDR_TYPE_STRING

    /* xdr_inxXX_t.c*/
    ,XDR_TYPE_INT8
    ,XDR_TYPE_INT16
    ,XDR_TYPE_INT32
    ,XDR_TYPE_INT64
    ,XDR_TYPE_UINT8
    ,XDR_TYPE_UINT16
    ,XDR_TYPE_UINT32
    ,XDR_TYPE_UINT64
    ,XDR_TYPE_FLOAT
    ,XDR_TYPE_DOUBLE
    ,XDR_TYPE_ARRAY
    ,XDR_TYPE_POINTER
};

typedef struct XDR XDR;

/*
 * A xdrproc_t exists for each data type which is to be encoded or decoded.
 *
 * The second argument to the xdrproc_t is a pointer to an opaque pointer.
 * The opaque pointer generally points to a structure of the data type
 * to be decoded.  If this pointer is 0, then the type routines should
 * allocate dynamic storage of the appropriate size and return it.
 * bool_t       (*xdrproc_t)(XDR *, caddr_t *);
 */
typedef bool_t (*xdrproc_t) (XDR *, void *,...);

/*
 * The XDR handle.
 * Contains operation which is being applied to the stream,
 * an operations vector for the particular implementation (e.g. see xdr_mem.c),
 * and two private fields for the use of the particular implementation.
 */
struct XDR
  {
    enum xdr_op x_op;		/* operation; fast additional param */
    struct xdr_ops
      {
	bool_t (*x_getlong) (XDR *__xdrs, long *__lp);
	/* get a long from underlying stream */
	bool_t (*x_putlong) (XDR *__xdrs, long *__lp);
	/* put a long to " */
	bool_t (*x_getbytes) (XDR *__xdrs, caddr_t __addr, u_int __len);
	/* get some bytes from " */
	bool_t (*x_putbytes) (XDR *__xdrs, char *__addr, u_int __len);
	/* put some bytes to " */
	u_int (*x_getpostn) (XDR *__xdrs);
	/* returns bytes off from beginning */
	bool_t (*x_setpostn) (XDR *__xdrs, u_int __pos);
	/* lets you reposition the stream */
	int32_t *(*x_inline) (XDR *__xdrs, u_int __len);
	/* buf quick ptr to buffered data */
	void (*x_destroy) (XDR *__xdrs);
	/* free privates of this xdr_stream */
	bool_t (*x_getint32) (XDR *__xdrs, int32_t *__ip);
	/* get a int from underlying stream */
	bool_t (*x_putint32) (XDR *__xdrs, int32_t *__ip);
	/* put a int to " */
	  
	  /* optional: push or pop an element in a struct, for XML */
	  void (*x_label)(XDR *__xdrs, bool_t push);
	  
	  /* optional: serialize the base types (int, string, array, etc) */
	  bool_t (*x_type)(XDR *xdr, enum xdr_type type, void *p, 
			   xdrproc_t eltproc, int *nelts, int eltsize, int maxelts);
      }
      *x_ops;
      caddr_t x_public;		/* users' data */
      caddr_t x_private;	/* pointer to private data */
      caddr_t x_base;		/* private used for position info */
      u_int x_handy;		/* extra private word */
      
      XDR_LABEL *x_label;
  };

void xdr_init(XDR *xdr);
void xdr_label_push(XDR *xdr, XDR_LABEL *label, char *name, char *type);
void xdr_label_pop(XDR *xdr);



/*
 * Operations defined on a XDR handle
 *
 * XDR          *xdrs;
 * int32_t      *int32p;
 * long         *longp;
 * caddr_t       addr;
 * u_int         len;
 * u_int         pos;
 */
#define XDR_GETINT32(xdrs, int32p)                      \
        (*(xdrs)->x_ops->x_getint32)(xdrs, int32p)
#define xdr_getint32(xdrs, int32p)                      \
        (*(xdrs)->x_ops->x_getint32)(xdrs, int32p)

#define XDR_PUTINT32(xdrs, int32p)                      \
        (*(xdrs)->x_ops->x_putint32)(xdrs, int32p)
#define xdr_putint32(xdrs, int32p)                      \
        (*(xdrs)->x_ops->x_putint32)(xdrs, int32p)

#define XDR_GETLONG(xdrs, longp)			\
	(*(xdrs)->x_ops->x_getlong)(xdrs, longp)
#define xdr_getlong(xdrs, longp)			\
	(*(xdrs)->x_ops->x_getlong)(xdrs, longp)

#define XDR_PUTLONG(xdrs, longp)			\
	(*(xdrs)->x_ops->x_putlong)(xdrs, longp)
#define xdr_putlong(xdrs, longp)			\
	(*(xdrs)->x_ops->x_putlong)(xdrs, longp)

#define XDR_GETBYTES(xdrs, addr, len)			\
	(*(xdrs)->x_ops->x_getbytes)(xdrs, addr, len)
#define xdr_getbytes(xdrs, addr, len)			\
	(*(xdrs)->x_ops->x_getbytes)(xdrs, addr, len)

#define XDR_PUTBYTES(xdrs, addr, len)			\
	(*(xdrs)->x_ops->x_putbytes)(xdrs, addr, len)
#define xdr_putbytes(xdrs, addr, len)			\
	(*(xdrs)->x_ops->x_putbytes)(xdrs, addr, len)

#define XDR_GETPOS(xdrs)				\
	(*(xdrs)->x_ops->x_getpostn)(xdrs)
#define xdr_getpos(xdrs)				\
	(*(xdrs)->x_ops->x_getpostn)(xdrs)

#define XDR_SETPOS(xdrs, pos)				\
	(*(xdrs)->x_ops->x_setpostn)(xdrs, pos)
#define xdr_setpos(xdrs, pos)				\
	(*(xdrs)->x_ops->x_setpostn)(xdrs, pos)

#define	XDR_INLINE(xdrs, len)				\
	(*(xdrs)->x_ops->x_inline)(xdrs, len)
#define	xdr_inline(xdrs, len)				\
	(*(xdrs)->x_ops->x_inline)(xdrs, len)

#define	XDR_DESTROY(xdrs)					\
	do {							\
		if ((xdrs)->x_ops->x_destroy)			\
			(*(xdrs)->x_ops->x_destroy)(xdrs);	\
	} while (0)
#define	xdr_destroy(xdrs)					\
	do {							\
		if ((xdrs)->x_ops->x_destroy)			\
			(*(xdrs)->x_ops->x_destroy)(xdrs);	\
	} while (0)

/*
 * Support struct for discriminated unions.
 * You create an array of xdrdiscrim structures, terminated with
 * a entry with a null procedure pointer.  The xdr_union routine gets
 * the discriminant value and then searches the array of structures
 * for a matching value.  If a match is found the associated xdr routine
 * is called to handle that part of the union.  If there is
 * no match, then a default routine may be called.
 * If there is no match and no default routine it is an error.
 */
#define NULL_xdrproc_t ((xdrproc_t)0)
struct xdr_discrim
{
  int value;
  xdrproc_t proc;
};

/*
 * Inline routines for fast encode/decode of primitive data types.
 * Caveat emptor: these use single memory cycles to get the
 * data from the underlying buffer, and will fail to operate
 * properly if the data is not aligned.  The standard way to use these
 * is to say:
 *      if ((buf = XDR_INLINE(xdrs, count)) == NULL)
 *              return (FALSE);
 *      <<< macro calls >>>
 * where ``count'' is the number of bytes of data occupied
 * by the primitive data types.
 *
 * N.B. and frozen for all time: each data type here uses 4 bytes
 * of external representation.
 */

#define IXDR_GET_INT32(buf)           ((int32_t)ntohl((uint32_t)*(buf)++))
#define IXDR_PUT_INT32(buf, v)        (*(buf)++ = (int32_t)htonl((uint32_t)(v)))
#define IXDR_GET_U_INT32(buf)         ((uint32_t)IXDR_GET_INT32(buf))
#define IXDR_PUT_U_INT32(buf, v)      IXDR_PUT_INT32(buf, (int32_t)(v))

/* WARNING: The IXDR_*_LONG defines are removed by Sun for new platforms
 * and shouldn't be used any longer. Code which use this defines or longs
 * in the RPC code will not work on 64bit Solaris platforms !
 */
#define IXDR_GET_LONG(buf) \
	((long)ntohl((u_long)*__extension__((u_int32_t*)(buf))++))
#define IXDR_PUT_LONG(buf, v) \
	(*__extension__((u_int32_t*)(buf))++ = (long)htonl((u_long)(v)))
#define IXDR_GET_U_LONG(buf)	      ((u_long)IXDR_GET_LONG(buf))
#define IXDR_PUT_U_LONG(buf, v)	      IXDR_PUT_LONG(buf, (long)(v))


#define IXDR_GET_BOOL(buf)            ((bool_t)IXDR_GET_LONG(buf))
#define IXDR_GET_ENUM(buf, t)         ((t)IXDR_GET_LONG(buf))
#define IXDR_GET_SHORT(buf)           ((short)IXDR_GET_LONG(buf))
#define IXDR_GET_U_SHORT(buf)         ((u_short)IXDR_GET_LONG(buf))

#define IXDR_PUT_BOOL(buf, v)         IXDR_PUT_LONG(buf, (long)(v))
#define IXDR_PUT_ENUM(buf, v)         IXDR_PUT_LONG(buf, (long)(v))
#define IXDR_PUT_SHORT(buf, v)        IXDR_PUT_LONG(buf, (long)(v))
#define IXDR_PUT_U_SHORT(buf, v)      IXDR_PUT_LONG(buf, (long)(v))

/*
 * These are the "generic" xdr routines.
 * None of these can have applied because it's not possible to
 * know whether the call is a read or a write to the passed parameter
 * also, the XDR structure is always updated by some of these calls.
 */
extern bool_t xdr_void (void) ;
extern bool_t xdr_short (XDR *__xdrs, short *__sp) ;
extern bool_t xdr_u_short (XDR *__xdrs, u_short *__usp) ;
extern bool_t xdr_int (XDR *__xdrs, int *__ip) ;
extern bool_t xdr_u_int (XDR *__xdrs, u_int *__up) ;
extern bool_t xdr_long (XDR *__xdrs, long *__lp) ;
extern bool_t xdr_u_long (XDR *__xdrs, u_long *__ulp) ;
extern bool_t xdr_hyper (XDR *__xdrs, quad_t *__llp) ;
extern bool_t xdr_u_hyper (XDR *__xdrs, u_quad_t *__ullp) ;
extern bool_t xdr_longlong_t (XDR *__xdrs, quad_t *__llp) ;
extern bool_t xdr_u_longlong_t (XDR *__xdrs, u_quad_t *__ullp) ;
extern bool_t xdr_int8_t (XDR *__xdrs, int8_t *__ip) ;
extern bool_t xdr_uint8_t (XDR *__xdrs, uint8_t *__up) ;
extern bool_t xdr_int16_t (XDR *__xdrs, int16_t *__ip) ;
extern bool_t xdr_uint16_t (XDR *__xdrs, uint16_t *__up) ;
extern bool_t xdr_int32_t (XDR *__xdrs, int32_t *__ip) ;
extern bool_t xdr_uint32_t (XDR *__xdrs, uint32_t *__up) ;
extern bool_t xdr_int64_t (XDR *__xdrs, int64_t *__ip) ;
extern bool_t xdr_uint64_t (XDR *__xdrs, uint64_t *__up) ;
extern bool_t xdr_bool (XDR *__xdrs, bool_t *__bp) ;
extern bool_t xdr_enum (XDR *__xdrs, enum_t *__ep) ;
extern bool_t xdr_array (XDR * _xdrs, caddr_t *__addrp, u_int *__sizep,
			 u_int __maxsize, u_int __elsize, xdrproc_t __elproc)
     ;
extern bool_t xdr_bytes (XDR *__xdrs, char **__cpp, u_int *__sizep,
			 u_int __maxsize) ;
extern bool_t xdr_opaque (XDR *__xdrs, caddr_t __cp, u_int __cnt) ;
extern bool_t xdr_string (XDR *__xdrs, char **__cpp, u_int __maxsize) ;
extern bool_t xdr_union (XDR *__xdrs, enum_t *__dscmp, char *__unp,
			 struct xdr_discrim *__choices,
			 xdrproc_t dfault) ;
extern bool_t xdr_char (XDR *__xdrs, char *__cp) ;
extern bool_t xdr_u_char (XDR *__xdrs, u_char *__cp) ;
extern bool_t xdr_vector (XDR *__xdrs, char *__basep, u_int __nelem,
			  u_int __elemsize, xdrproc_t __xdr_elem) ;
extern bool_t xdr_float (XDR *__xdrs, float *__fp) ;
extern bool_t xdr_double (XDR *__xdrs, double *__dp) ;
extern bool_t xdr_reference (XDR *__xdrs, caddr_t *__xpp, u_int __size,
			     xdrproc_t __proc) ;
extern bool_t xdr_pointer (XDR *__xdrs, char **__objpp,
			   u_int __obj_size, xdrproc_t __xdr_obj) ;
extern bool_t xdr_wrapstring (XDR *__xdrs, char **__cpp) ;
extern u_long xdr_sizeof (xdrproc_t, void *) ;

/*
 * Common opaque bytes objects used by many rpc protocols;
 * declared here due to commonality.
 */
#define MAX_NETOBJ_SZ 1024
struct netobj
{
  u_int n_len;
  char *n_bytes;
};
typedef struct netobj netobj;
extern bool_t xdr_netobj (XDR *__xdrs, struct netobj *__np) ;

/*
 * These are the public routines for the various implementations of
 * xdr streams.
 */

/* XDR using memory buffers */
extern void xdrmem_create (XDR *__xdrs, caddr_t __addr,
			   u_int __size, enum xdr_op __xop) ;

/* XDR using stdio library */
extern void xdrstdio_create (XDR *__xdrs, FILE *__file, enum xdr_op __xop)
     ;

/* XDR pseudo records for tcp */
extern void xdrrec_create (XDR *__xdrs, u_int __sendsize,
			   u_int __recvsize, caddr_t __tcp_handle,
			   int (*__readit) (char *, char *, int),
			   int (*__writeit) (char *, char *, int)) ;

/* make end of xdr record */
extern bool_t xdrrec_endofrecord (XDR *__xdrs, bool_t __sendnow) ;

/* move to beginning of next record */
extern bool_t xdrrec_skiprecord (XDR *__xdrs) ;

/* true if no more input */
extern bool_t xdrrec_eof (XDR *__xdrs) ;

/* free memory buffers for xdr */
extern void xdr_free (xdrproc_t __proc, void *__objp) ;


/* write XML to xdr_out */
int
xdrxml_create_xdr_write(XDR *xdr, XDR *xdr_out);

/* parse XML from buf */
int
xdrxml_create_buf_read(XDR *xdr, char *buf, int len);

/* dump x to buf, return length of buf.  this wraps <root>...</root>
   around the dumped structure, unless omit_root=1, which is required
   for xdrxml_parse to work.  if omit_root=0, you must wrap the output
   in an xml tag for xdrxml_parse to work. */
int
xdrxml_fmt(xdrproc_t proc, void *x, char *buf, int len, int omit_root);

/* parse buf and put in x.  call xdr_free(proc, x) to free it */
int
xdrxml_parse(xdrproc_t proc, void *x, char *buf, int len);

/* parse XML from a stream */
typedef int (*xdrxml_stream_recv_func_t)(XDR *xdr, void *farg);

int
xdrxml_stream_open(XDR *xdr, xdrxml_stream_recv_func_t func, void *farg);

void
xdrxml_stream_recv(XDR *xdr, char *buf, int len, int final);

void
xdrxml_stream_close(XDR *xdr);

/* quickly pack to/from a buffer */
int
xdr_pack(xdrproc_t proc, void *x, enum xdr_op op, enum xdr_mode  mode,
	 char *buf, int len);

#endif /* rpc/xdr.h */
