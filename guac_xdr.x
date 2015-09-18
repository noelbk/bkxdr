/* -*- c -*- */
#ifndef GUAC_XDR_X_INCLUDED
#define GUAC_XDR_X_INCLUDED

enum guac_hunk_type {
    GUAC_HUNK_SURFACE_RESIZE=1
    ,GUAC_HUNK_SURFACE_DRAW
    ,GUAC_HUNK_SURFACE_PAINT
    ,GUAC_HUNK_SURFACE_COPY
    ,GUAC_HUNK_SURFACE_TRANSFER
    ,GUAC_HUNK_SURFACE_RECT
    ,GUAC_HUNK_SURFACE_CLIP
    ,GUAC_HUNK_SURFACE_RESET_CLIP
    ,GUAC_HUNK_SURFACE_FLUSH
    ,GUAC_HUNK_SURFACE_FLUSH_DEFERRED

    ,GUAC_HUNK_CLIPBOARD_ALLOC
    ,GUAC_HUNK_CLIPBOARD_FREE
    ,GUAC_HUNK_CLIPBOARD_SEND
    ,GUAC_HUNK_CLIPBOARD_RESET
    ,GUAC_HUNK_CLIPBOARD_APPEND

    ,GUAC_HUNK_SET_DOT_CURSOR
    ,GUAC_HUNK_SET_POINTER_CURSOR
};

typedef int guac_surface_id_t;

struct guac_surface_buf_t {
    int w;
    int h;
    opaque buf<>;
};


struct guac_surface_draw_t {
    guac_surface_id_t surface;
    int x;
    int y;
    guac_surface_buf_t src;
};

struct guac_surface_resize_t {
    guac_surface_id_t surface;
    int w;
    int h;
};

struct guac_surface_paint_t {
    guac_surface_id_t surface;
    int x;
    int y;
    guac_surface_buf_t src;
    int red;
    int green;
    int blue;
};

union guac_hunk_t switch (guac_hunk_type type) {
 case GUAC_HUNK_SURFACE_RESIZE:  guac_surface_resize_t surface_resize;
 case GUAC_HUNK_SURFACE_DRAW:    guac_surface_draw_t surface_draw;
 case GUAC_HUNK_SURFACE_PAINT:   guac_surface_paint_t surface_paint;
 };

#endif /* GUAC_XDR_X_INCLUDED */


