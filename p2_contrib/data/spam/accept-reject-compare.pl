#!/usr/bin/perl -w
use strict;
use POSIX;
use POSIX qw(strftime);
use Getopt::Std;
use Time::Local;
use Digest::SHA1  qw(sha1 sha1_hex sha1_base64);

my $minargs = 4;
my $maxargs = 4;
my $argusage = "<anonymized-log-dir> <date-start> <date-end> <comp-file>\n Dates should be in format (in quotes due to spaces) \"YEAR MONTH DAY HR MIN SEC\" \n Start date is inclusive. End date is not inclusive";
sub usage {
    die "Usage: $0 $argusage\n" .
        "";
}


my %flags = ();
getopts("", \%flags);
usage() if (@ARGV < $minargs or @ARGV > $maxargs);

my $anonDir = shift;
my $timeS = shift;
my $timeE = shift;
my $compFile = shift;

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

my %IPMapA = ();
my %IPMapR = ();

my $numIPA = 0;
my $numIPR = 0;

opendir(DIR, $anonDir) or die "cant open pipe $anonDir: $!";
my @files = grep(/.*\.log$/,readdir(DIR));
closedir(DIR);

my $filename;

foreach $filename (@files) {
    if ($filename =~ /anon-maillog\.(\d{4})(\d{2})(\d{2})\.log/) {
        my $date = join('-', ($1, $2, $3));
        my $logStartTime = timegm(0, 0, 0, $3, $2-1, $1);
        my $logEndTime = timegm(59, 59, 23, $3, $2-1, $1);

        if($logStartTime >= $startTime || $logEndTime>=$startTime) {
            print STDERR "processing $filename\n";
            
            my $absPath = $anonDir . $filename;
            open (PIPE, "$absPath") or die "cant open pipe $filename: $!";
            
            while(<PIPE>) {
                my @vecWords = split(/\s+/, $_);
                my $curCond = $vecWords[0];
    
                my @days = split(/-/, $vecWords[1]);
                my @times = split(/:/, $vecWords[2]);
    
                my $cYear = $days[0];
                my $cMonth = $days[1];
                my $cDay = $days[2];
                my $cHour = $times[0];
                my $cMin = $times[1];
                my $cSec = $times[2];

                my $curTime = timegm($cSec, $cMin, $cHour, $cDay, $cMonth-1, $cYear);

                #check if in current time frame
                next unless (($curTime >= $startTime && $curTime < $endTime && $curCond eq "Accepted") || ($curTime >= $endTime && $curCond eq "Rejected"));
            
                my $hashValue = $vecWords[3];
                if ($curCond eq "Accepted")
                {
                    if (!exists($IPMapA{$hashValue}))
                    {
                        # $numIPA++;
                        $IPMapA{$hashValue} = gmtime($curTime);
                        # $numIPA;
                    }
                }
                if ($curCond eq "Rejected")
                {
                    if (!exists($IPMapR{$hashValue}))
                    {
                        # $numIPR++;
                        $IPMapR{$hashValue} = gmtime($curTime);
                    }
                }
            }
        }
        close PIPE;
    }
}


open(COMP, ">$compFile") or die("Cannot write file: $compFile");

while((my $key, my $value) = each %IPMapA)
{
    if(exists($IPMapR{$key})) {
        print COMP "Accepted = " , $key, " Time = ", $value, " Rejected Time= ", $IPMapR{$key}, "\n";
    }
}
close COMP;

