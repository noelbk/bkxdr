/*
 *  bkxml.h - routines for quickly parsing XML to a tree, uses bklib
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

#ifndef BKXML_H_INCLUDED
#define BKXML_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef struct xml_node_t {
    struct xml_node_t *parent, *child, *last_child, *next;
    char *name;
    char *value;
    int  len;
} xml_node_t;

struct xml_t {
    xml_node_t *root;    /* the root of the whole tree */
    xml_node_t *current; /* the current node I'm filling (below the root) */
    int depth;

    int err_code;
    int err_line;
    char *err_string;

    void *internal;
};

typedef struct xml_t xml_t;

void
xml_delete(xml_t *xml);


xml_t *
xml_new();

int
xml_parse(xml_t *xml, char *buf, int len, int eof);

xml_node_t *
xml_find(xml_t *xml, char *path);

char*
xml_find_val(xml_t *xml, char *path);

xml_node_t *
xml_node_find(xml_node_t *root, char *path);

int
xml_node_get_val(xml_node_t *node, char *path, char *buf, int len);

int
xml_node_dump(xml_node_t *root, int indent, char *buf, int len);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // BKXML_H_INCLUDED
