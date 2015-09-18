/*
 *  bkxml_t.c - test bkxml.c
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
#include <string.h>
#include "bkxml.h"

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

int main() {
    xml_t *xml=0;
    xml_node_t *node;
    int i;
    char buf[4096];

    do {
	xml = xml_new();
	i = strlen(xml_str);
	xml_parse(xml, xml_str, i, 1);

	xml_node_dump(xml->root, 0, buf, sizeof(buf));
	printf("%s", buf);

	node = xml_node_find(xml->root->child, "rec_classroom/students/elt");
	for(; node; node=node->next) {
	    xml_node_dump(node, 0, buf, sizeof(buf));
	    printf("%s", buf);
	}
	
    } while(0);
    if( xml ) {
	xml_delete(xml);
    }
}

