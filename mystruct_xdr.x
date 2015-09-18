/* -*- C -*- */
/* mystruct_xdr.x */

struct MyStruct {
    string name<>;          /* null-terminated string */
    float vector<>;         /* variable-length array  */
    opaque binary<>;        /* binary blob */
    struct MyStruct *next;  /* linked lists are ok! */
};
typedef struct MyStruct MyStruct;

