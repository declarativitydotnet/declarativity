#!/usr/bin/perl 
# Create a Chord network exponentially, with degree d.  First gateway,
# then d nodes off of that, then d^2 nodes off of those, etc.

# Once it is all set up, start running random lookups from random nodes

use strict 'vars';
use Getopt::Std;
use FindBin;

our($opt_f);

if ($#ARGV == -1){
    print "Usage buildCDF.pl [options]\n";
    print "options:\n";
    print "\t-f <inputFile> : File containing a lookup delay measurement per line\n";
    exit 0;
}

if (!getopts('f:')){
    die "Unknown option specified\n";
}

if (!defined $opt_f) {
    die "Must have an input file name";
}



open INFILE, "<$opt_f" or die "Couldn't open $opt_f";
my @array = ();
while (<INFILE>) {
	chomp;
	my @line = split / /;
	my $latency = $line[0];
	my $hopCount = $line[1]; 
	$array[$hopCount] = $array[$hopCount] + 1;
}

my $size = @array;
my $count = 0;
for ($count = 1; $count < $size; $count++) {
    print "$count $array[$count]\n";
}

close INFILE;


#End
