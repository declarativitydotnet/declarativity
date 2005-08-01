#!/usr/bin/perl 

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



my $latencyCounts = 0;
my $totalLatencyCounts = 0;

open INFILE, "<$opt_f" or die "Couldn't open $opt_f";
while (<INFILE>) {
    chomp;
    $totalLatencyCounts += 1;
}

close INFILE;

print "Total number of lookups " . $totalLatencyCounts . "\n";

open INFILE, "<$opt_f" or die "Couldn't open $opt_f";
my $totalLatency = 0;
my $medianLatency = -1;
while (<INFILE>) {
	chomp;
	my @line = split / /;
	my $latency = $line[0];
	$latencyCounts++;
	$totalLatency += $latency;
	my $percentCount = 0;
	$percentCount = ($latencyCounts / $totalLatencyCounts) * 100;
	if ($percentCount > 50 && $medianLatency == -1) {
	    $medianLatency = $latency;
	}
	print "$latency $latencyCounts $percentCount\n";
}
my $meanLatency = $totalLatency / $totalLatencyCounts;
print "Mean = $meanLatency\n";
print "Median = $medianLatency\n";

close INFILE;


#End
