#!/usr/local/bin/perl

#
# this program fixes a bug with bibtex's handling of long lines.
# It finds all lines which end in a % sign, removes the %, and
# merges those lines with the next line
#

#$x = <>;
#print $x;
#print "\\addtolength{\\itemsep}{-1ex}\n";

while(<>) {
    chop;
    if(/\%$/) {
	chop;
	$x = <>;
	printf "%s%s", $_, $x;
    } else {
	print "$_\n";
    }
}
