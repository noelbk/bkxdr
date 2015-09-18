#include <stdio.h>
#include "rpc/xdr.h"

#define assertb(cond) \
if( !(cond) ) { \
   fprintf(stderr, "%s:%d: assert(" #cond ") failed\n", __FILE__, __LINE__); \
   break; \
}


int
main() {
    char buf[4096];
    double x, y;
    int i, err=-1;

    do {
	x = 3.14;
	i = xdr_pack((xdrproc_t)xdr_double, &x, XDR_ENCODE, XDR_BINARY, buf, sizeof(buf));
	assertb(i>0);
	i = xdr_pack((xdrproc_t)xdr_double, &y, XDR_DECODE, XDR_BINARY, buf, sizeof(buf));
	assertb(i>0);
	printf("xdr_double: x=%g y=%g i=%d\n", x, y, i);

	err =0;
    } while(0);
    return err;
}
