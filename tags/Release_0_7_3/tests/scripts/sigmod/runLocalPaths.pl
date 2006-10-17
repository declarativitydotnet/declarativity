#!/usr/bin/perl 

use strict 'vars';
use Getopt::Std;
use FindBin;

our($opt_n);
our($opt_d);
our($opt_f);
our($opt_D);
our($opt_p);
our($opt_m);
our($opt_i);
our($opt_u);
our($opt_c);
our($opt_r);
our($opt_s);
our($opt_q);
our($opt_C);
our($opt_P);

if ($#ARGV == -1){
    print "Usage runLocalPaths.pl [options]\n";
    print "options:\n";
    print "\t-n <numNodes> : Number of nodes\n";
    print "\t-d <dir> : Output Directory\n";
    print "\t-f <dir> : Input Graph File\n";
    print "\t-D <duration>:  Duration\n";
    print "\t-p <periodic>:  Periodic Push Results\n";
    print "\t-m <metric> : Metric\n";
    print "\t-i <file> : Datalog file\n";
    print "\t-u <periodic update> : Periodic update\n";
    print "\t-c <update change percent> : Periodic change percent\n";
    print "\t-r <flag>: 0 - no correlation, 1 - correlation\n";
    print "\t-s <magicSets interval>: -1 no magic sets\n";
    print "\t-q <magic num queries>: number of queries\n";
    print "\t-C <magic cache>: 0 - no use cache, 1 - use cache\n";
    print "\t-P <percent magic dsts>\n";
    exit 0;
}

if (!getopts('n:d:f:D:p:m:i:u:c:r:s:q:C:P:')){
    die "Unknown option specified\n";
}

if (!defined $opt_n) {
    die "Must have number of nodes";
}

if (!defined $opt_r) {
    die "Must have correlation";
}

if (!defined $opt_u) {
    die "Must have periodic update";
}

if (!defined $opt_c) {
    die "Must have periodic change percent";
}

if (!defined $opt_d) {
    die "Must have output directory";
}

if (!defined $opt_f) {
    die "Must have input graph";
}

if (!defined $opt_p) {
    die "Must have periodic";
}

if (!defined $opt_D) {
    die "Must have query duration";
}

if (!defined $opt_m) {
    die "Must have metric";
}

if (!defined $opt_i) {
    die "Must have datalog file";
}

if (!defined $opt_s) {
    die "Must have magic sets interval";
}

if (!defined $opt_q) {
    die "Must have number of queries";
}

if (!defined $opt_C) {
    die "Must have magic cache flag";
}

if (!defined $opt_P) {
    die "Must have percent magic dsts";
}

my $count = 0;
system("rm -rf $opt_d.out.*");
system("rm -rf $opt_d-*");
for ($count = 0; $count < $opt_n; $count++)
{
    my $port = 20000 + $count;
    my $execStr = "./tests/paths $opt_n $count 127.0.0.1 $port $opt_i $opt_f $opt_d NONE $count $opt_D $opt_p $opt_m $opt_u $opt_c $opt_r $opt_s $opt_q $opt_C $opt_P >& $opt_d.out.$count &";
    #$execStr = "valgrind --tool=addrcheck $execStr"; 
    print "Execute $execStr\n";
    system($execStr);
}
