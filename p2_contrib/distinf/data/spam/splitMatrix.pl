#!/usr/bin/perl -w
#

use strict;
use POSIX;
use POSIX qw(strftime);
use Getopt::Std;
use Time::Local;

my $minargs = 3;
my $maxargs = 3;
my $argusage = "<sample-file> <count> <basename>";
sub usage {
    die "Usage: $0 $argusage\n" .
        "";
}


my %flags = ();
getopts("", \%flags);
usage() if (@ARGV < $minargs or @ARGV > $maxargs);

my $sampleFile = shift;
my $count = shift;
my $basename = shift;


print "$count\n"; 

my @words =  split(/\//, $sampleFile);
my @fName = split(/\./,$words[2]);
my $name = $basename.$fName[0]."_".$count.".dat";
open(PART, ">$name") or die("Cannot read file: $name");

open(SAMPLE, "<$sampleFile") or die("Cannot read file: $sampleFile");
while(<SAMPLE>)
{
   my @vecWords = split(/\s+/, $_);
   my $rowno = $vecWords[0];
   if($rowno <= $count)
   {
       print PART "@vecWords\n";  
   }  
}
close SAMPLE;
close PART;
