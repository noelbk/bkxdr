#include <string.h>
#include "xdr_xml_t_xdr.h"


char *xml =
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
"      <name type=\"string\">Beth</name>\n"
"      <age type=\"int\">40</age>\n"
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
func(XDR *xdr, void *farg) {
    struct rec_classroom c;
    xdr_rec_classroom(xdr, &c);

    printf("received rec_classroom students=%d student=%s\n"
	   ,c.students.students_len
	   ,c.students.students_val[0].name
	   );

    xdr_free((xdrproc_t)xdr_rec_classroom, (char*)&c);
    return 0;
}

int main() {
    XDR xdrs, *xdr=&xdrs;
    char *p;

    xdrxml_stream_open(xdr, func, 0);
    for(p=xml; *p; p++) {
	xdrxml_stream_recv(xdr, p, 1, (p+1)==0);
    }
    xdrxml_stream_close(xdr);
    return 0;
}
