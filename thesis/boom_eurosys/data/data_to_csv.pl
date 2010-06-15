#!/usr/bin/perl -w
use strict;

my @lines = <>;

my @cols;

my $i = 0;
my $max_len = 0;
foreach my $line (@lines) {
    chomp $line;
    $line =~ s/^(.*)\s*\[/$1,/;
    $line =~ s/\]\s*$//;
    my @tok = split /,\s*/, $line;
    $cols[$i] = \@tok;
    $i++;
    $max_len = ($max_len >= @tok) ? $max_len : (0+@tok);
}

for(my $i = 0; $i < $max_len; $i++) {
    print (($i+1) * 0.01);
    foreach my $col (@cols) {
        if(defined($$col[$i])) {
            print ",$$col[$i]";
        } else {
            print ",1.000";
        }
    }
    print "\n";
}
