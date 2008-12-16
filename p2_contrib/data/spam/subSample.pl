#!/usr/bin/perl -w
#

use strict;
use Getopt::Std;
use List::Util 'shuffle';
use Data::Dumper;

my $minargs = 4;
my $maxargs = 4;
my $argusage = "fileIn fileOut rowMap numSamples ";
sub usage {
    die "Usage: $0 $argusage\n" .
        "";
}


my %flags = ();
getopts("", \%flags);
usage() if (@ARGV < $minargs or @ARGV > $maxargs);

my $fileIn = shift;
my $fileOut = shift;
my $fileMap = shift;
my $numSamples = shift;
my $usedlist = shift;

open(FIN, "<$fileIn") or die("Cannot read file: $fileIn");
open(FOUT, ">$fileOut") or die("Cannot write file: $fileOut");
`touch $fileMap`;
open(MAPOUT, "<$fileMap") or die("Cannot read file: $fileMap");

my @file = ();
my @fileNum = ();
my $curNum = 1;
my @usedNums = ();

my $itNum = 0;
while(<MAPOUT>)
{
    my @numList = split(/ /, $_);
    push(@usedNums, $numList[1]);
    $itNum = $numList[2];
}
close MAPOUT;
@usedNums = sort {$a <=> $b} @usedNums;

$itNum++;

my $curUsedIndex = 0;
while(<FIN>)
{
    push(@file, $_);
    while ($curUsedIndex <= $#usedNums && $curNum > $usedNums[$curUsedIndex])
    {
	$curUsedIndex++;
    }
    if ($curUsedIndex > $#usedNums || $curNum < $usedNums[$curUsedIndex])
    {
	push(@fileNum, $curNum);
    }
    else
    {
	#curNum == so just increment the index, but we want to skip this element
	$curUsedIndex++;
    }
    $curNum++;
}
close FIN;

my @fNShuffled = shuffle(@fileNum);

open(MAPOUT, ">>$fileMap") or die("Cannot append file: $fileMap");

if ($#fNShuffled+1 < $numSamples)
{
    print "ERROR: Not enough unused samples\n";
    exit(1);
}
for(my $i = 0; $i < $numSamples; $i++)
{
    if (!defined($file[$fNShuffled[$i]-1]))
    {
	print "$i, $fNShuffled[$i]\n";
	print "$#file\n";
    }
    print FOUT $file[$fNShuffled[$i]-1];
    my $mI = $i + 1;
    my $mSI = $fNShuffled[$i];
    print MAPOUT "$mI $mSI $itNum\n";
}
close MAPOUT;
close FOUT;
