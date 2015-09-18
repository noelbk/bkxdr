/*
 *  xdr_xml_t_xdr.x - test XDR typedefs for xdr_t.c
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

typedef string strul<>;

struct rec_person {
    string name<>;
    int age;
    float weight;
    rec_person *next;
};

struct rec_classroom {
    string name<>;
    rec_person students<>;
};

enum rec_type {
    REC_PERSON=1,
    REC_CLASSROOM=2
};

union rec switch(rec_type type) {
 case REC_PERSON: rec_person person;
 case REC_CLASSROOM: rec_classroom classroom;
 default: void;
};

