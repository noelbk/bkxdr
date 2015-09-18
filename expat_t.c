#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "expat.h"

typedef struct xdrxml_node_t {
    struct xdrxml_node_t *parent, *child, *last_child, *next;
    char *name;
    char *value;
} xdrxml_node_t;

typedef struct xdrxml_userdata_t {
    xdrxml_node_t *root;    /* the root of the whole tree */
    xdrxml_node_t *current; /* the current node I'm filling (below the root) */
} xdrxml_userdata_t;

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

static void
xdrxml_start_element(void *userData, const char *name, const char **atts) {
    xdrxml_userdata_t *ctx = (xdrxml_userdata_t *)userData;
    xdrxml_node_t *current = ctx->current;
    xdrxml_node_t *child;
    
    child = calloc(sizeof(*child), 1);
    child->parent = current;
    child->name = strdup(name);
    
    if( !ctx->root ) {
	ctx->root = child;
    }
    else {
	if( current->last_child ) {
	    child->next = current->last_child->next;
	    current->last_child->next = child;
	}
	else {
	    current->child = child;
	}
	current->last_child = child;
    }
    ctx->current = child;
}

static void
xdrxml_end_element(void *userData, const char *name) {
    xdrxml_userdata_t *ctx = (xdrxml_userdata_t *)userData;
    xdrxml_node_t *current = ctx->current;

    ctx->current = current->parent;
}

static void
xdrxml_char_data(void *userData, const XML_Char *value, int len) {
    xdrxml_userdata_t *ctx = (xdrxml_userdata_t *)userData;
    xdrxml_node_t *current = ctx->current;
    int i, n;

    i = current->value ? strlen(current->value) : 0;
    current->value = realloc(current->value, i+len+1);
    strncpy(current->value+i, value, len);
    current->value[i+len] = 0;
}

char *xml_str =
"<root>\n"
"<rec_classroom>\n"
"  <name type=\"string\">Class 1</name>\n"
"  <students type=\"rec_person\">\n"
"    <elt index=\"0\">\n"
"      <name type=\"string\">Noel</name>\n"
"      <age type=\"int\">34</age>\n"
"      <weight type=\"float\">190.000000</weight>\n"
"      <next type=\"rec_person\"></next>\n"
"    </elt>\n"
"    <elt index=\"1\">\n"
"      <name type=\"string\">Jack</name>\n"
"      <age type=\"int\">8</age>\n"
"      <weight type=\"float\">42.000000</weight>\n"
"      <next type=\"rec_person\">\n"
"        <name type=\"string\">Noel</name>\n"
"        <age type=\"int\">34</age>\n"
"        <weight type=\"float\">190.000000</weight>\n"
"        <next type=\"rec_person\"></next>\n"
"      </next>\n"
"    </elt>\n"
"  </students>\n"
"</rec_classroom>\n"
"\n"
"<rec_classroom>\n"
"  <name type=\"string\">Class 1</name>\n"
"  <students type=\"rec_person\">\n"
"    <elt index=\"0\">\n"
"      <name type=\"string\">Noel</name>\n"
"      <age type=\"int\">34</age>\n"
"      <weight type=\"float\">190.000000</weight>\n"
"      <next type=\"rec_person\"></next>\n"
"    </elt>\n"
"    <elt index=\"1\">\n"
"      <name type=\"string\">Jack</name>\n"
"      <age type=\"int\">8</age>\n"
"      <weight type=\"float\">42.000000</weight>\n"
"      <next type=\"rec_person\">\n"
"        <name type=\"string\">Noel</name>\n"
"        <age type=\"int\">34</age>\n"
"        <weight type=\"float\">190.000000</weight>\n"
"        <next type=\"rec_person\"></next>\n"
"      </next>\n"
"    </elt>\n"
"  </students>\n"
"</rec_classroom>\n"
"\n"
"<rec_classroom>\n"
"  <extra_field></extra_field>\n"
"  <missing_name/>\n"
"  <students type=\"rec_person\">\n"
"    <elt index=\"0\">\n"
"      <missing_name/>\n"
"      <age type=\"int\">34</age>\n"
"      <missing_weight/>\n"
"      <next type=\"rec_person\"></next>\n"
"    </elt>\n"
"    <elt index=\"1\">\n"
"      <name type=\"string\">Jack</name>\n"
"      <age type=\"int\">8</age>\n"
"      <weight type=\"float\">42.000000</weight>\n"
"      <next type=\"rec_person\">\n"
"        <name type=\"string\">Noel</name>\n"
"        <age type=\"int\">34</age>\n"
"        <weight type=\"float\">190.000000</weight>\n"
"        <next type=\"rec_person\"></next>\n"
"      </next>\n"
"    </elt>\n"
"  </students>\n"
"</rec_classroom>\n"
"</root>\n"
;

int
main(int argc, char *argv[])
{
  XML_Parser parser;
  xdrxml_userdata_t ctx;
  int  i;

  parser = XML_ParserCreate(NULL);
  memset(&ctx, 0, sizeof(ctx));
  XML_SetUserData(parser, &ctx);
  XML_SetElementHandler(parser, xdrxml_start_element, xdrxml_end_element);
  XML_SetCharacterDataHandler(parser, xdrxml_char_data);

  i = XML_Parse(parser, xml_str, strlen(xml_str), 1);
  if( i == XML_STATUS_ERROR ) {
      fprintf(stderr,
	      "%s at line %d\n",
	      XML_ErrorString(XML_GetErrorCode(parser)),
	      XML_GetCurrentLineNumber(parser));
  }
  XML_ParserFree(parser);

  xdrxml_node_print(ctx.root, 0);

  xdrxml_node_free(ctx.root);

  return 0;
}

