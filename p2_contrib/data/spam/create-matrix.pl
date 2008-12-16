#!/usr/bin/perl -w
#

use strict;
use POSIX;
use POSIX qw(strftime);
use Getopt::Std;
use Time::Local;

my $minargs = 6;
my $maxargs = 6;
my $argusage = "<anonymized-list> <date-start> <date-end> <rejected-matrix-out> <accepted-matrix-out> <mapping-out>\n Dates should be in format (in quotes due to spaces) \"YEAR MONTH DAY HR MIN SEC\" \n Start date is inclusive. End date is not inclusive";
sub usage {
    die "Usage: $0 $argusage\n" .
        "";
}


my %flags = ();
getopts("", \%flags);
usage() if (@ARGV < $minargs or @ARGV > $maxargs);

my $anonList = shift;
my $timeS = shift;
my $timeE = shift;
my $rmatFile = shift;
my $amatFile = shift;
my $mapFile = shift;

my @timeSVec = split(/\s+/, $timeS);
my @timeEVec = split(/\s+/, $timeE);



my $sYear = $timeSVec[0];
my $sMonth = $timeSVec[1];
my $sDay = $timeSVec[2];
my $sHour = $timeSVec[3];
my $sMin = $timeSVec[4];
my $sSec = $timeSVec[5];


my $eYear = $timeEVec[0];
my $eMonth = $timeEVec[1];
my $eDay = $timeEVec[2];
my $eHour = $timeEVec[3];
my $eMin = $timeEVec[4];
my $eSec = $timeEVec[5];



my $startTime = timegm($sSec, $sMin, $sHour, $sDay, $sMonth-1, $sYear);
my $endTime = timegm($eSec, $eMin, $eHour, $eDay, $eMonth-1, $eYear);

print "startTime = ", strftime("%d %m %Y %H:%M:%S", gmtime($startTime)), "\n";
print "endTime = ", strftime("%d %m %Y %H:%M:%S", gmtime($endTime)), "\n";

open(ANON, "<$anonList") or die("Cannot read file: $anonList");

my %domainMap = ();;
my %IPMapA = ();
my %IPMapR = ();

my $numIPA = 0;
my $numIPR = 0;
my $numDom = 0;
my %matrix = ();
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

 
    #check if in current time frame
    next unless (($curTime >= $startTime && $curTime < $endTime && $curCond eq "Rejected") || ($curTime >= $endTime && $curCond eq "Accepted"));
    
    my $hashValue = $vecWords[3];
    my $domainValue = $vecWords[4];
    if ($curCond eq "Rejected")
    {
        if (!exists($IPMapR{$hashValue}))
        {
            $numIPR++;
            $IPMapR{$hashValue} = $numIPR;
        }
    }
    if ($curCond eq "Accepted")
    {
        if (!exists($IPMapA{$hashValue}))
        {
            $numIPA++;
            $IPMapA{$hashValue} = $numIPA;
        }
    }
    if (!exists($domainMap{$domainValue}))
    {
        $numDom++;
        $domainMap{$domainValue} = $numDom;
    }
    
    my $IPnum = ($curCond eq "Rejected") ? $IPMapR{$hashValue} : $IPMapA{$hashValue};
    my $domainNum = $domainMap{$domainValue};
    my $matKey = "$IPnum $domainNum $curCond";
    if (exists($matrix{$matKey}))
    {
    $matrix{$matKey}++;
    }
    else
    {
    $matrix{$matKey} = 1;
    }
}

close ANON;

open(MATID, ">$rmatFile") or die("Cannot write file: $rmatFile");
for(my $i = 1; $i <= $numIPR; $i++)
{
    for(my $j = 1; $j <= $numDom; $j++)
    {
        my $matKey = "$i $j Rejected";
        my $value = exists($matrix{$matKey}) ? $matrix{$matKey} : 0;
        print MATID "$value ";
    }
    print MATID "\n";
} 
close MATID;

open(AMATID, ">$amatFile") or die("Cannot write file: $amatFile");
for(my $i = 1; $i <= $numIPA; $i++)
{
    for(my $j = 1; $j <= $numDom; $j++)
    {
        my $matKey = "$i $j Accepted";
        my $value = exists($matrix{$matKey}) ? $matrix{$matKey} : 0;
        print AMATID "$value ";
    }
    print AMATID "\n";
} 
close AMATID;

open(MAP, ">$mapFile") or die("Cannot write file: $mapFile");
while((my $key, my $value) = each %IPMapR)
{
    print MAP "$key $value Rejected \n";
}
while((my $key, my $value) = each %IPMapA)
{
    print MAP "$key $value Accepted \n";
}
while((my $key, my $value) = each %domainMap)
{
    print MAP "$key $value\n";
}
close MAP;
