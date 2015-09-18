#include <string.h>

#include "guac_xdr.h"

void
xml_dump(xdrproc_t p, void *v) {
    XDR xdr_out, xdr_xml;
    xdrstdio_create(&xdr_out, stdout, XDR_ENCODE);
    xdrxml_create_xdr_write(&xdr_xml, &xdr_out);
    p(&xdr_xml, v);
    xdr_destroy(&xdr_xml);
}

int
main() {
    guac_hunk_t hunk;

    printf("\nsurface_resize:");
    hunk.type = GUAC_HUNK_SURFACE_RESIZE; 
    {
	guac_surface_resize_t *p = &(hunk.guac_hunk_t_u.surface_resize);
	p->surface = 1;
	p->w = 100;
	p->h = 200;
    }
    xml_dump((xdrproc_t)xdr_guac_hunk_t, &hunk);

    printf("\nsurface_draw:");
    hunk.type = GUAC_HUNK_SURFACE_DRAW; 
    {
	guac_surface_draw_t *p = &(hunk.guac_hunk_t_u.surface_draw);
	p->surface = 1;
	p->x = 100;
	p->y = 200;
	p->src.w = 10;
	p->src.h = 20;
	p->src.buf.buf_val = "1234";
	p->src.buf.buf_len = strlen(p->src.buf.buf_val);
    }
    xml_dump((xdrproc_t)xdr_guac_hunk_t, &hunk);

    return 0;
}
