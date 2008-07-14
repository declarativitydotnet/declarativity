#!/usr/bin/perl 

use strict 'vars';
use Getopt::Std;
use FindBin;

our($opt_f);
our($opt_n);

if ($#ARGV == -1){
    print "Usage buildCDF.pl [options]\n";
    print "options:\n";
    print "\t-f <inputFile> : File containing a lookup delay measurement per line\n";
    print "\t-n <lookups> : Number of simultaneous lookups\n";
    exit 0;
}

if (!getopts('f:n:')){
    die "Unknown option specified\n";
}

if (!defined $opt_f) {
    die "Must have an input file name";
}

if (!defined $opt_n) {
    die "Must have number of lookups";
}

my $totalLatencyCounts = 0;

open INFILE, "<$opt_f/allLookups.out" or die "Couldn't open $opt_f";

# go through the file, get <IP address, search key, located key>
# index each search key by all located keys separated by ":"
# go through each search key, for each key, count the size
# pick largest, divide by N

my %searchKeys;
while (<INFILE>) {
	chomp;
	my @line = split /, /;
	$searchKeys{$line[2]} = "";	
    }
close INFILE;

open INFILE, "<$opt_f/simple_results" or die "Couldn't open $opt_f";
while (<INFILE>) {
    chomp;
    my @line = split /, /;
    my $str = $searchKeys{$line[4]};
    $str = $str . " " . $line[5];
    $searchKeys{$line[4]} = $str;
}
close INFILE;

# go through all keys
my $key;
my $totalSize = 0;
my @consistencyArray = ();
foreach $key (keys (%searchKeys)) {
    #print $key . " => " . $searchKeys{$key} . "\n";
    my @array = split(/ /, $searchKeys{$key});
    my $x = 0;
    my $y = 0;
    my $size = @array;
    my $max = 0;
    for ($x = 0; $x < $size; $x++) {
	my $count = 0;
	my $str1 = $array[$x];	
	for ($y = 0; $y < $size; $y++) {	    
		my $str2 = $array[$y];
		if ($str1 == $str2) {
		    $count++;		
		}
	}
	if ($count > $max) {
	    $max = $count * 1.0;
	}
    }
    if ($max > $opt_n) {
	$max = $opt_n;
    } 
    $consistencyArray[$max] = $consistencyArray[$max] + 1;
    $totalSize += 1;
}

my $x = 0;
my $cum = 0;
for ($x = 0; $x < @consistencyArray; $x++) {
    if ($consistencyArray[$x] == undef) {
	$consistencyArray[$x] = 0;
    }
    print $x . " " . $consistencyArray[$x] . " " . $x / $opt_n . " " . $cum / $totalSize . "\n";
    $cum = $cum + $consistencyArray[$x];
    print $x . " " . $consistencyArray[$x] . " " . $x / $opt_n . " " . $cum / $totalSize . "\n";
}

