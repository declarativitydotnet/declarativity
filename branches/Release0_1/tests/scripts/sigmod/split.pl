#!/usr/bin/perl 

use strict 'vars';
use Getopt::Std;
use FindBin;

our($opt_n);
our($opt_p);
our($opt_s);

if ($#ARGV == -1){
    print "Usage runLocalPaths.pl [options]\n";
    print "options:\n";
    print "\t-n <numNodes> : Number of nodes\n";
    print "\t-p <prefix> : Base prefix\n";
    #print "\t-s <split> : Number of queries to split into\n";
}


if (!getopts('n:p:')){
    die "Unknown option specified\n";
}

#go through all files with prefix, do a grep and output
my $count = 0;
for ($count = 0; $count < $opt_n; $count++) {
    print "grep Zero, $opt_p-$count > $opt_p-0-$count\n";
    system("grep Zero, $opt_p-$count > $opt_p-0-$count");
    print "grep One, $opt_p-$count > $opt_p-1-$count\n";
    system("grep One, $opt_p-$count > $opt_p-1-$count");
    print "grep Two, $opt_p-$count > $opt_p-2-$count\n";
    system("grep Two, $opt_p-$count > $opt_p-2-$count");
}



