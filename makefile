#
#  makefile - unix makefile for bkxdr
#  Noel Burton-Krahn <noel@burton-krahn.com>
#  Jan 1, 2003
#
#  Copyright (C) Noel Burton-Krahn, 2004, and is released under the
#  GPL version 2 (see below).
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program (see the file COPYING included with this
#  distribution); if not, write to the Free Software Foundation, Inc.,
#  59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#

INC=-I. -Iexpat-1.95.8/lib

#---------------------------------------------------------------------
# unix
l = .a
o = .o
x = 
CFLAGS = -Wall -Wno-unused -Wno-pointer-sign -D_GNU_SOURCE -DOS=OS_UNIX -g $(INC) # -O2 -pg
LFLAGS = -g # -O2 -pg
LINK_OUT = $(CC) $(LFLAGS) -o $@
MAKE_CMD = $(MAKE) -$(MAKEFLAGS) 
.SUFFIXES = .c .h .o .dep
LINK_LIB = ld -r -o $@
LINK_EXE = $(CC) $(LFLAGS) -o $@

RPCGEN=./rpcgen$x

libxdr_obj_os=-L. -lexpat

default: all

libexpat$l: expat-1.95.8
	cd expat-1.95.8 && ./configure && make
	cp expat-1.95.8/.libs/libexpat$l .

libbk_objs=../bklib/libbk$l
libbk_libs=-lpthread

include makefile.common
