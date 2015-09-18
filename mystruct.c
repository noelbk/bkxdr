/* mystruct.c */
#include "mystruct_xdr.h"

int
main(int argc, char **argv) {
    MyStruct orig1, orig2, copy;
    float vector1[] = {1.1f, 2.2f, 3.3f};
    char  binary1[]  = {0xde, 0xad, 0xbe, 0xef};
    char buf[4096];
    int len;

    /* set up a cople of linked structures, orig1 and orig2 */
    memset(&orig1, 0, sizeof(orig1));
    orig1.name = "orig1";
    orig1.vector.vector_val = vector1;
    orig1.vector.vector_len = sizeof(vector1)/sizeof(*vector1);	
    orig1.binary.binary_val = binary1;
    orig1.binary.binary_len = sizeof(binary1)/sizeof(*binary1);	

    memset(&orig2, 0, sizeof(orig2));
    orig2.name = "orig2";
    orig1.next = &orig2;

    /* serialize orig1 and linked orig2 to buf	 */
    len = xdr_pack(xdr_MyStruct, &orig1, XDR_ENCODE, XDR_BINARY, 
		   buf, sizeof(buf));
    printf("encoded orig1 to binary len=%d\n", len);

    /* now you can save buf to disk, or fire it over the net */

    /* deserialize buf to copy */
    memset(&copy, 0, sizeof(copy));
    xdr_pack(xdr_MyStruct, &copy, XDR_DECODE, XDR_BINARY, 
	     buf, len);

    /* copy contains malloced data, so you must free it */
    xdr_free(xdr_MyStruct, &copy);

    /* the same serialization, but this time to/from xml */
    len = xdr_pack(xdr_MyStruct, &orig1, XDR_ENCODE, XDR_XML, 
		   buf, sizeof(buf));
    printf("encoded orig1 to xml len=%d\n%s\n", len, buf);

    memset(&copy, 0, sizeof(copy));
    xdr_pack(xdr_MyStruct, &copy, XDR_DECODE, XDR_XML, 
	     buf, len);

    len = xdr_pack(xdr_MyStruct, &copy, XDR_ENCODE, XDR_XML, 
		   buf, sizeof(buf));
    printf("encoded copy to xml len=%d\n%s\n", len, buf);

    xdr_free(xdr_MyStruct, &copy);
    
    return 0;
}

/*
  compile-command: cl -Zip -W3 -I.. -o mystruct mystruct.c mystruct_xdr.c ../libxdr.lib
 */

