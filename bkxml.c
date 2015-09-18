/*
 *  bkxml.c - routines for quickly parsing XML to a tree
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define XML_STATIC 
#include "expat.h"

#include "bkxml.h"
#include "defutil.h"
#include "config.h"

#define parser(xml) ((XML_Parser)xml->internal)
#define parser_set(xml, val) (xml->internal = (XML_Parser)val)

#define assertb(cond) if( !(cond) ) { break; }

void
xml_node_free(xml_node_t *root) {
    xml_node_t *child, *next;
    for(child=root->child; child; child=next) {
	next = child->next;
	xml_node_free(child);
    }
    if( root->name ) free(root->name);
    if( root->name ) free(root->value);
    free(root);
}

int
xml_node_dump(xml_node_t *root, int indent, char *buf, int len) {
    xml_node_t *child, *next;
    char *orig = buf;
    int i, l;

    do {
	for(i=0; i<indent; i++) {
	    l = snprintf(buf, len, " ");
	    BUF_ADD(buf, len, l);
	}
	l = snprintf(buf, len, "<%s>", root->name);
	BUF_ADD(buf, len, l);

	if( root->len ) {
	    l = snprintf(buf, len, "%s", root->value);
	    BUF_ADD(buf, len, l);
	}

	if( root->child ) {
	    l = snprintf(buf, len, "\n");
	    BUF_ADD(buf, len, l);
	}

	for(child=root->child; child; child=next) {
	    next = child->next;
	    l = xml_node_dump(child, indent+1, buf, len);
	    BUF_ADD(buf, len, l);
	}

	if( root->child ) {
	    for(i=0; i<indent; i++) {
		l = snprintf(buf, len, " ");
		BUF_ADD(buf, len, l);
	    }
	}

	l = snprintf(buf, len, "</%s>\n", root->name);
	BUF_ADD(buf, len, l);

    } while(0);
    return buf-orig;
}

xml_node_t*
xml_node_new_child(xml_node_t *parent, const char *name) {
    xml_node_t *child;
    
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
xml_start_element(void *userData, const char *name, const char **atts) {
    xml_t *xml = (xml_t*)userData;
    xml_node_t *current = xml->current;
    xml_node_t *child;
    
    child = xml_node_new_child(current, name);
    if( !xml->root ) {
	xml->root = child;
    }
    xml->current = child;
    xml->depth++;
}

void
xml_end_element(void *userData, const char *name) {
    xml_t *xml = (xml_t*)userData;
    xml_node_t *current = xml->current;
    char *p;

    /* trim trailing spaces */
    for(p = current->value + current->len-1;
	current->len>0 && isspace(*p); current->len--, p--) {
	*p = 0;
    }

    xml->current = current->parent;
    xml->depth--;
}

void
xml_char_data(void *userData, const XML_Char *value, int len) {
    xml_t *xml = (xml_t*)userData;
    xml_node_t *current = xml->current;
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

void
xml_delete(xml_t *xml) {
    if( xml ) {
	if( parser(xml) ) {
	    XML_ParserFree(parser(xml));
	    parser_set(xml, 0);
	}
	if( xml->root ) {
	    xml_node_free(xml->root);
	    xml->root = 0;
	}
	free(xml);
    }
}


xml_t*
xml_new() {
    xml_t *xml = 0;
    int err=-1;
    do {
	xml = (xml_t *)calloc(1, sizeof(*xml));
	assertb(xml);
	
	memset(xml, 0, sizeof(*xml));
	parser_set(xml, XML_ParserCreate(NULL));
	assertb(parser(xml));
	
	XML_SetElementHandler(parser(xml), xml_start_element, xml_end_element);
	XML_SetCharacterDataHandler(parser(xml), xml_char_data);
	XML_SetUserData(parser(xml), xml);
	err = 0;
    } while(0);
    if( err ) {
	xml_delete(xml);
	xml = 0;
    }
    return xml;
}

int
xml_parse(xml_t *xml, char *buf, int len, int eof) {
    int i;
    i = XML_Parse(parser(xml), buf, len, eof);
    if( i == XML_STATUS_ERROR ) {
	xml->err_code = XML_GetErrorCode(parser(xml));
	xml->err_string = (char*)XML_ErrorString(xml->err_code);
	xml->err_line = XML_GetCurrentLineNumber(parser(xml));
	return -1;
    }
    return 0;
}

xml_node_t *
xml_node_find(xml_node_t *root, char *path) {
    xml_node_t *child;
    char *slash;
    int n;

    child=root;
    if( *path == '/' ) { path++; }
    while(child && *path) {
	slash = strchr(path, '/');
	if( slash ) {
	    n = slash-path;
	}
	else {
	    n = strlen(path);
	}
	for(; child; child=child->next) {
	    if( strncmp(child->name, path, n) == 0 && !child->name[n]) {
		if( slash ) {
		    /* descend into this child */
		    child = child->child;
		    path = slash+1;
		    break;
		}
		else {
		    return child;
		}
	    }
	}
    }
    return 0;
}

xml_node_t *
xml_find(xml_t *xml, char *path) {
    return xml_node_find(xml->root, path);
}

char*
xml_find_val(xml_t *xml, char *path) {
    xml_node_t *node;
    node = xml_node_find(xml->root, path);
    if( node ) {
	return node->value;
    }
    return 0;
}




