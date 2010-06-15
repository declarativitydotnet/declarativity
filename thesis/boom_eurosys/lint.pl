#!/usr/bin/perl -w 

# complain about bad macro invocations

use strict;

foreach my $f (@ARGV) { 
    open IN, $f;
    $.= 0;
    while(my $line = <IN>) {
        if($line =~ /\\([a-z]+)\\/i || $line =~ /includegraphics.+in\]/) {
            if (!defined($1) || ($1 ne "hline" && $1 ne "def" && $1 ne "let")) {
                if($line !~ /^\\newcommand/) {
                    print "$f:$.: $line";
                }
            }
        }
    }
}
