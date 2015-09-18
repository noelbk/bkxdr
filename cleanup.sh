#! /bin/sh

set -eux

find . -name '*.[ch]' | xargs perl -pni.bak -e '
BEGIN {

undef $/;

$s = join("|", qw(
__BEGIN_DECLS
__END_DECLS
__const
__THROW
const
__BEGIN_NAMESPACE_STD
__END_NAMESPACE_STD
));
}

s/($s)[ \t]*//og;
s/^#define[ \t]*\n//g;
'

