#include "rpc/xdr.h"
#include <stdlib.h>
#include <string.h>

void
xdr_init(XDR *xdr) {
    memset(xdr, 0, sizeof(*xdr));
}
    
void
xdr_label_push(XDR *xdr, XDR_LABEL *label, char *name, char *type) {
    memset(label, 0, sizeof(*label));
    label->name = name;
    label->type = type;
    label->depth = xdr->x_label ? xdr->x_label->depth+1 : 1;
    label->next = xdr->x_label;
    xdr->x_label = label;

    if( xdr->x_ops && xdr->x_ops->x_label ) {
	xdr->x_ops->x_label(xdr, 1);
    }
}

void
xdr_label_pop(XDR *xdr) {
    if( xdr->x_ops && xdr->x_ops->x_label ) {
	xdr->x_ops->x_label(xdr, 0);
    }
    if( xdr->x_label ) {
	xdr->x_label = xdr->x_label->next;
    }
}

