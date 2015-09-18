/*
 *  xdr_xml_t.c - test program for bkxdr
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

#include "xdr_xml_t_xdr.h"
#include <string.h>

void
xml_dump(xdrproc_t p, void *v) {
    XDR xdr_out, xdr_xml;
    xdrstdio_create(&xdr_out, stdout, XDR_ENCODE);
    xdrxml_create_xdr_write(&xdr_xml, &xdr_out);
    p(&xdr_xml, v);
    xdr_destroy(&xdr_xml);
}

int main() {
    struct rec_person p[2];
    struct rec_classroom c;
    struct rec r;

    printf("\nrec_person:");
    memset(&p[0], 0, sizeof(p[0]));
    p[0].name = "Noel";
    p[0].age  = 34;
    p[0].weight = 190;
    xml_dump((xdrproc_t)xdr_rec_person, &p[0]);

    printf("\n\nrec_person:");
    memset(&p[1], 0, sizeof(p[1]));
    p[1].name = "Jack";
    p[1].age  = 8;
    p[1].weight = 42;
    p[1].next = &p[0];
    xml_dump((xdrproc_t)xdr_rec_person, &p[1]);
     
    printf("\n\nrec_classroom:");
    memset(&c, 0, sizeof(c));
    c.name = "Class 1";
    c.students.students_val = p;
    c.students.students_len = 2; 
    xml_dump((xdrproc_t)xdr_rec_classroom, &c);
   
    printf("\n\nrec(REC_CLASSROOM):");
    memset(&r, 0, sizeof(r));
    r.type = REC_CLASSROOM;
    r.rec_u.classroom = c;
    xml_dump((xdrproc_t)xdr_rec, &r);

    printf("\n\nrec(REC_PERSON):");
    memset(&r, 0, sizeof(r));
    r.type = REC_PERSON;
    r.rec_u.person = p[0];
    xml_dump((xdrproc_t)xdr_rec, &r);
    
    return 0;
}
