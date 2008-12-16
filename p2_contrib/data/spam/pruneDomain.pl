#!/usr/bin/perl -w
#

use strict;
use POSIX;
use POSIX qw(strftime);
use Getopt::Std;
use Time::Local;

my $minargs = 3;
my $maxargs = 3;
my $argusage = "<anonymized-dir> <pruneDomain> <allDomain>";
sub usage {
    die "Usage: $0 $argusage\n" .
        "";
}


my %flags = ();
getopts("", \%flags);
usage() if (@ARGV < $minargs or @ARGV > $maxargs);

my $anonDir = shift;
my $pruneDomain = shift;
my $allDomain = shift;


opendir(DH, $anonDir)        or die "Couldn't open $anonDir for reading: \n";

my @files = ();
my @dirFiles = readdir(DH);
foreach my $file(@dirFiles) 
{
    if (!(($file eq ".") || ($file eq "..")))
    {
	my $filename = "$anonDir/$file";
	push(@files, $filename) unless -d $filename;
    }
}

my %domainCount = ();;

my $numDom = 0;

print "Parsing Domains\n";
my @files2 = @files;
foreach(@files)
{   
    print "Parsing $_ \n";
    open(ANON, "<$_") or die("Cannot read file: $_");
    while(<ANON>)
    {
    	my @vecWords = split(/\s+/, $_);
    	my $curCond = $vecWords[0];

        # next unless ($curCond eq "Rejected");
    	next unless ($#vecWords == 4);
    
    	my $domainValue = $vecWords[4];
    	if (!exists($domainCount{$domainValue}))
    	{    	 
    	    $domainCount{$domainValue} = "1";
    	}
    	else
    	{
            $domainCount{$domainValue} = $domainCount{$domainValue} + 1;
        }
    }

    close ANON;
}


open(MAP, ">$allDomain") or die("Cannot write file: $allDomain");
while((my $key, my $value) = each %domainCount)
{
    print MAP "$key $value \n";
}
close MAP;

open(MAP, ">$pruneDomain") or die("Cannot write file: $pruneDomain");
while((my $key, my $value) = each %domainCount)
{
    if($value > 50)
    {
        $numDom++;
        print MAP "$key $numDom \n";
    }
}
close MAP;
