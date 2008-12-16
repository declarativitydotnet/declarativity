#!/usr/bin/perl -w
#

use strict;
use POSIX;
use POSIX qw(strftime);
use Getopt::Std;
use Time::Local;

my $minargs = 6;
my $maxargs = 6;
my $argusage = "<anonymized-dir> <domain> <basename-out> <mapping-out> <stats> <top-n>";
sub usage {
    die "Usage: $0 $argusage\n" .
        "";
}

my %flags = ();
getopts("", \%flags);
usage() if (@ARGV < $minargs or @ARGV > $maxargs);

my $anonDir = shift;
my $domains = shift;
my $basename = shift;
my $mapFile = shift;
my $stats = shift;
my $domCount = shift;

opendir(DH, $anonDir)        or die "Couldn't open $anonDir for reading: \n";

my @files = ();
my @dirFiles = readdir(DH);
foreach my $file(@dirFiles) 
{
    # if (!(($file eq ".") || ($file eq "..") || ($file eq ".DS_Store")))
    # {
    #     my $filename = "$anonDir/$file";
    #     # print "$filename \n";
    #     push(@files, $filename) unless -d $filename;
    # }
    
    if ($file eq "anon-maillog.20070301.log")
    {
      my $filename = "$anonDir/$file";
      push(@files, $filename) unless -d $filename;
    }
}


my %domainMap = ();;
my $n = "";
my $numDom = 0;
my $name = "";

# print "Loading Domains\n";
my @files2 = @files;
open(DOM, "<$domains") or die("Cannot read file: $domains");

while(<DOM>)
{   
    my @vecWords = split(/\s+/, $_);
    my $domainValue = $vecWords[2];
    if (!exists($domainMap{$domainValue}))
    {
      $numDom++;
      $domainMap{$domainValue} = "$numDom";
    }
    if($numDom == $domCount){
        last;
    }
}
close DOM;

print "Total domains: $numDom \n";
foreach my $key (sort {$domainMap{$a} <=> $domainMap{$b}} keys %domainMap){ 
     print "$domainMap{$key} $key\n";
}


foreach(@files2)
{
    print "Parsing $_ \n";
    createMatrix($_, $basename, $mapFile);
}

sub createMatrix
{

    my ($anonList, $basename, $mapFile) = @_;

    my %IPMapR1 = ();
    my %IPMapR2 = ();
    my %IPMapR3 = ();
    my %IPMapR4 = ();

    my $numIPR1 = 0;
    my $numIPR2 = 0;
    my $numIPR3 = 0;
    my $numIPR4 = 0;
    
    my %matrix1 = ();
    my %matrix2 = ();
    my %matrix3 = ();
    my %matrix4 = ();
    
    my @fileNameSplit = split(/\./, $anonList);
    my $baseFile = $fileNameSplit[1];
    
    my $rFile1 = $basename . "_" . $baseFile . "_06.dat";
    my $rFile2 = $basename . "_" . $baseFile . "_12.dat";
    my $rFile3 = $basename . "_" . $baseFile . "_18.dat";
    my $rFile4 = $basename . "_" . $baseFile . "_24.dat";
    
    open(ANON, "<$anonList") or die("Cannot read file: $anonList");
    while(<ANON>)
    {
	    my @vecWords = split(/\s+/, $_);
    	my $curCond = $vecWords[0];

    	my @days = split(/-/, $vecWords[1]);
    	my @times = split(/:/, $vecWords[2]);

    	# print @days, " ", @times, "\n";

    	my $cYear = $days[0];
    	my $cMonth = $days[1];
    	my $cDay = $days[2];
    	my $cHour = $times[0];
    	my $cMin = $times[1];
    	my $cSec = $times[2];

    	my $curTime = timegm($cSec, $cMin, $cHour, $cDay, $cMonth-1, $cYear);

    	my $satisfy1 = $cHour < 6;
    	my $satisfy2 = $cHour < 12 && $cHour >= 6;
    	my $satisfy3 = $cHour < 18 && $cHour >= 12;
    	my $satisfy4 = $cHour < 24 && $cHour >= 18;
    
    	#check if in current time frame
        # next unless ($curCond eq "Rejected");
        next unless ($curCond eq "Rejected");
    	next unless ($#vecWords == 4);
    
    	my $hashValue = $vecWords[3];
    	my $domainValue = $vecWords[4];
    
    	next unless (exists($domainMap{$domainValue}));
    	my $domainNum = $domainMap{$domainValue};
    
    	if ($satisfy1)
    	{
    	    if (!exists($IPMapR1{$hashValue}))
    	    {
    		$numIPR1++;
    		$IPMapR1{$hashValue} = $numIPR1;
    	    }
    	    my $IPnum = $IPMapR1{$hashValue};
    	    my $matKey = "$IPnum $domainNum";
    	    if (exists($matrix1{$matKey}))
    	    {
    		$matrix1{$matKey}++;
    	    }
    	    else
    	    {
    		$matrix1{$matKey} = 1;
    	    }
    	}
        if ($satisfy2)
        {
            if (!exists($IPMapR2{$hashValue}))
            {
                $numIPR2++;
                $IPMapR2{$hashValue} = $numIPR2;
            }
            my $IPnum = $IPMapR2{$hashValue};
            my $matKey = "$IPnum $domainNum";
            if (exists($matrix2{$matKey}))
            {
              $matrix2{$matKey}++;
            }
            else
            {
              $matrix2{$matKey} = 1;
            }
        }
        if ($satisfy3)
        {
            if (!exists($IPMapR3{$hashValue}))
            {
                $numIPR3++;
                $IPMapR3{$hashValue} = $numIPR3;
            }
            my $IPnum = $IPMapR3{$hashValue};
            my $matKey = "$IPnum $domainNum";
            if (exists($matrix3{$matKey}))
            {
                $matrix3{$matKey}++;
            }
            else
            {
                $matrix3{$matKey} = 1;
            }
        }
        if ($satisfy4)
        {
            if (!exists($IPMapR4{$hashValue}))
            {
                $numIPR4++;
                $IPMapR4{$hashValue} = $numIPR4;
            }
            my $IPnum = $IPMapR4{$hashValue};
            my $matKey = "$IPnum $domainNum";
            if (exists($matrix4{$matKey}))
            {
                $matrix4{$matKey}++;
            }
            else
            {
                $matrix4{$matKey} = 1;
            }
        }
    }
    close ANON;
    
    print "Generating 00-06 hour dataset\n";
    open(MATID, ">$rFile1") or die("Cannot write file: $rFile1");
    for(my $i = 1; $i <= $numIPR1; $i++)
    {
        for(my $j = 1; $j <= $numDom; $j++)
        {
          my $matKey = "$i $j";
          my $value = exists($matrix1{$matKey}) ? $matrix1{$matKey} : 0;
          if($value)
          {
              # print "$i, $j \n";
              print MATID "$i $j $value \n";
          }
        }
        # print MATID "\n";
    } 
    close MATID;
    
    print "Generating 06-12 hour dataset\n";
    open(MATID, ">$rFile2") or die("Cannot write file: $rFile2");
    for(my $i = 1; $i <= $numIPR2; $i++)
    {
        for(my $j = 1; $j <= $numDom; $j++)
        {
            my $matKey = "$i $j";
            my $value = exists($matrix2{$matKey}) ? $matrix2{$matKey} : 0;
            if($value)
            {
                # print "$i, $j \n";
                print MATID "$i $j $value \n";
            }
        }
        # print MATID "\n";
    } 
    close MATID;
    
    print "Generating 12-18 hour datasets\n";
    open(MATID, ">$rFile3") or die("Cannot write file: $rFile3");
    for(my $i = 1; $i <= $numIPR3; $i++)
    {
        for(my $j = 1; $j <= $numDom; $j++)
        {
            my $matKey = "$i $j";
            my $value = exists($matrix3{$matKey}) ? $matrix3{$matKey} : 0;
            if($value)
            {
                # print "$i, $j \n";
                print MATID "$i $j $value \n";
            }
        }
        print MATID "\n";
    } 
    close MATID;
    
    print "Generating 18-24 hour datasets\n";
    open(MATID, ">$rFile4") or die("Cannot write file: $rFile4");
    for(my $i = 1; $i <= $numIPR4; $i++)
    {
        for(my $j = 1; $j <= $numDom; $j++)
        {
            my $matKey = "$i $j";
            my $value = exists($matrix4{$matKey}) ? $matrix4{$matKey} : 0;
            if($value)
            {
                # print "$i, $j \n";
                print MATID "$i $j $value \n";
            }
        }
        print MATID "\n";
    } 
    close MATID;
    
    open(MAP, ">>$mapFile") or die("Cannot write file: $mapFile");
    while((my $key, my $value) = each %IPMapR1)
    {
        print MAP "$key $value $rFile1 \n";
    }
    while((my $key, my $value) = each %IPMapR2)
    {
        print MAP "$key $value $rFile2 \n";
    }
    while((my $key, my $value) = each %IPMapR3)
    {
        print MAP "$key $value $rFile3 \n";
    }
    while((my $key, my $value) = each %IPMapR4)
    {
        print MAP "$key $value $rFile4 \n";
    }
    close MAP;
    
    open(STATS, ">>$stats") or die("Cannot write file: $stats");
    $n = "$baseFile" . "06";
    print STATS "$n $numIPR1 \n";
    $n = "$baseFile" . "12";
    print STATS "$n $numIPR2 \n";
    $n = "$baseFile" . "18";
    print STATS "$n $numIPR3 \n";
    $n = "$baseFile" . "24";
    print STATS "$n $numIPR4 \n"; 
    close STATS;
}
