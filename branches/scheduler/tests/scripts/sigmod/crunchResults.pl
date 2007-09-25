#!/usr/bin/perl 

#plot convergence time (eventual results vs time)
#plot average path latency vs time
#plot bandwidth usage over time 

use strict 'vars';
use Getopt::Std;
use FindBin;

our($opt_n);
our($opt_p);
our($opt_d);
our($opt_t);
our($opt_a);
our($opt_r);

if ($#ARGV == -1){
    print "Usage crunchResults.pl [options]\n";
    print "options:\n";
    print "\t-n <numNodes> : Number of nodes\n";
    print "\t-p <prefix> : log prefix\n";
    print "\t-d <dir> : Directory\n";
    print "\t-t <dir> : Time granularity\n";
    print "\t-a <agg type> : 0 - min, 1 - max\n";
    print "\t-r <correlate sharing> : 0 - no sharing, 1 - sharing\n";
    exit 0;
}

if (!getopts('n:p:d:t:a:r:')){
    die "Unknown option specified\n";
}

if (!defined $opt_n) {
    die "Must have num nodes";
}

if (!defined $opt_p) {
    die "Must have prefix";
}

if (!defined $opt_d) {
    die "Must have an input directory";
}

if (!defined $opt_t) {
    die "Must have input time granularity";
}

if (!defined $opt_a) {
    die "Must have agg type";
}

if (!defined $opt_r) {
    die "Must have correlated sharing";
}

# go through each log file, for each node, figure out how many best paths reported,
# figure out when is the time the eventual best path is received, 
# store all the best paths values from each node
open BW, ">$opt_d/$opt_p-bw" || die "Cannot open $opt_d/$opt_p-bw";
open BW1, ">$opt_d/$opt_p-bw-1" || die "Cannot open $opt_d/$opt_p-bw-1";

my $numNodes = $opt_n;
my $count = 0;
my %pathCount;
my %bestPaths;
my %totalBandwidth;
my %totalBandwidth1;

if ($opt_r == 1) {
    goto BANDWIDTH;
}
for ($count = 0; $count < $numNodes; $count++) {
    $pathCount{$count} = 0;
    my $nextFile = $opt_d . "/" . $opt_p . "-" . $count;
    open NODEDIR, $nextFile || die "Cannot open $nextFile";
    my $nodeFile;
    while ($nodeFile = <NODEDIR>) {    
	if (index($nodeFile, "bestPath") == -1) {
	    next;
	}
	if (index($nodeFile, "Insert") == -1) {
	    next;
	}
	chomp;
	my @line = split(/, /, $nodeFile);
	my $src = $line[4];
	my $dst = $line[5];
	my $cost = $line[6];
	$pathCount{$count} += 1;
	my $searchStr = $src . "|" . $dst;
	if ($bestPaths{$searchStr} == undef) {
	    $bestPaths{$searchStr} = $cost;
	    next;
	}
	if ($opt_a == "0" && $bestPaths{$searchStr} > $cost) {
	    $bestPaths{$searchStr} = $cost;
	    #print "$searchStr $cost\n";
	}
	if ($opt_a == "1" && $bestPaths{$searchStr} < $cost) {
	    $bestPaths{$searchStr} = $cost;
	    #print "$searchStr $cost\n";
	}
	#print "$src $dst $cost\n";
    }
}

my @keys = keys(%bestPaths);
my $keySize = @keys;
print "Number of best paths: $keySize\n";

my %timeDiscovered;
my %lastFoundCost;
my %pathVectors;
# plot best path discovered vs time 
open PATHVECTORS, ">$opt_d/$opt_p-pv" || die "Cannot open $opt_d/$opt_p-pv";
open PC, ">$opt_d/$opt_p-pc" || die "Cannot open $opt_d/$opt_p-pc";
open BC, ">$opt_d/$opt_p-bc" || die "Cannot open $opt_d/$opt_p-bc";
for ($count = 0; $count < $numNodes; $count++) {
    $pathCount{$count} = 0;
    my $nextFile = $opt_d . "/" . $opt_p . "-" . $count;
    open NODEDIR, $nextFile || die "Cannot open $nextFile";
    my $nodeFile;
    my $initialTime = -1;
    while ($nodeFile = <NODEDIR>) {    
	if (index($nodeFile, "bestPath") == -1) {
	    next;
	}
	if (index($nodeFile, "Insert") == -1) {
	    next;
	}
	chomp;
	my @line = split(/, /, $nodeFile);
	my $seconds = $line[1];
	my $src = $line[4];
	my $dst = $line[5];
	my $cost = $line[6];
	my $whereStart = index($nodeFile, "<", 100);
	my $whereEnd = index($nodeFile, ">", 100);

	my $searchStr = $src . "|" . $dst;	
	if ($cost != $bestPaths{$searchStr}) {
	    next;
	}
	if ($lastFoundCost{$searchStr} != undef && 
	    $lastFoundCost{$searchStr} == $cost) {
	    #print "Duplicate $src $dst $cost\n";
	    next;
	}
	$lastFoundCost{$searchStr} = $cost;
	@line = split(/\]/, $line[2]);

	my $nanoseconds = $line[0] / 1000000000;
	my $time = ($seconds + $nanoseconds) / $opt_t;

	#print "$seconds $line[0] $nanoseconds $time\n";
	if ($initialTime == -1) {
	    $initialTime = $time;
	}
	my $timeLapsed = int($time - $initialTime);
	if ($timeDiscovered{$timeLapsed} == undef) {
	    $timeDiscovered{$timeLapsed} = 1;
	} else {
	    $timeDiscovered{$timeLapsed} += 1;
	}       
	my $pathVector = substr($nodeFile, $whereStart, $whereEnd - $whereStart + 1);
	@pathVectors{$searchStr} = $pathVector;
	print PATHVECTORS "$src $dst $cost $pathVector\n";
	#print "$src $dst $pathVector\n";
	#print "$timeLapsed $timeDiscovered{$timeLapsed} $src $dst $cost\n";
    }
}
close PATHVECTORS;

# plot the best paths discovered over time
@keys = keys(%timeDiscovered);
$keySize = @keys;
my $cum = 0;
for ($count = 0; $count < $keySize; $count++) {
    $cum += $timeDiscovered{$count};
    my $time = $count * $opt_t;
    my $percentile = $cum / ($numNodes * ($numNodes-1));
    print "Best Path Results: $time $cum $timeDiscovered{$count} $percentile\n";
    print BC "Best Path Results: $time $cum $timeDiscovered{$count} $percentile\n";
}
close(BC);


# plot average best path computed vs time
my %pathsAtTime;

my $maxTime = -1;
# plot best path discovered vs time 
for ($count = 0; $count < $numNodes; $count++) {
    $pathCount{$count} = 0;
    my $nextFile = $opt_d . "/" . $opt_p . "-" . $count;
    open NODEDIR, $nextFile || die "Cannot open $nextFile";
    my $nodeFile;
    my $initialTime = -1;
    while ($nodeFile = <NODEDIR>) {    
	if (index($nodeFile, "<bestPath") == -1) {
	    next;
	}
	if (index($nodeFile, "Insert") == -1) {
	    next;
	}
	chomp;
	my @line = split(/, /, $nodeFile);
	my $seconds = $line[1];
	my $src = $line[4];
	my $dst = $line[5];
	my $cost = $line[6];

	my $searchStr = $src . "|" . $dst;	
	@line = split(/\]/, $line[2]);

	my $nanoseconds = $line[0] / 1000000000;
	my $time = ($seconds + $nanoseconds) / $opt_t;
	if ($initialTime == -1) {
	    $initialTime = $time;
	} 
	my $elapsedTime = int($time - $initialTime);
	if ($maxTime < $elapsedTime) { $maxTime = $elapsedTime; }

	my $nextStr = $searchStr . "|" . $cost;

	#print "$time $src $dst $cost\n";
	if ($pathsAtTime{$elapsedTime} == undef) {
	    $pathsAtTime{$elapsedTime} = $nextStr;
	    #print "$elapsedTime $pathsAtTime{$elapsedTime} $nextStr\n";
	} else {
	    $pathsAtTime{$elapsedTime} = $pathsAtTime{$elapsedTime} . "," . $nextStr;
	    #print "$elapsedTime $pathsAtTime{$elapsedTime} $nextStr\n";
	}
       #print "$timeLapsed $timeDiscovered{$timeLapsed} $src $dst $cost\n";
    }
}

#print "Max time $maxTime\n";

@keys = keys(%pathsAtTime);
$keySize = @keys;
my %currentPathCost;
for ($count = 0; $count < $maxTime; $count++) {
    my $nextStr = $pathsAtTime{$count};

    if ($nextStr == undef) {
	next;
    }

    my @line = split(/,/, $nextStr);
    my $numEntries = @line;
    #print "$count $numEntries\n";
    
    my $count1 = 0;
    for ($count1 = 0; $count1 < $numEntries; $count1++) {
	my @fields = split(/\|/, $line[$count1]);
	my $src = $fields[0];
	my $dst = $fields[1];
	my $cost = $fields[2];
	
	my $searchStr = $src . "|" . $dst;
	if ($currentPathCost{$searchStr} == undef) {
	    $currentPathCost{$searchStr} = $cost;
	} else {
	    #print "New best path $currentPathCost{$searchStr} $cost\n";
	    $currentPathCost{$searchStr} = $cost;
	}
    }
    #compute average    
    my $currentKeySize = keys(%currentPathCost);
    my $total = 0;
    my $key;
    foreach $key (keys (%currentPathCost)) {
	$total += $currentPathCost{$key};
    }
    my $avg = $total / $currentKeySize;
    my $curTime = $count * $opt_t;
    print "Current Path Length $curTime $avg $currentKeySize\n";
    print PC "Current Path Length $curTime $avg $currentKeySize\n";
}
close PC;

BANDWIDTH:
my $maxTime = 0;

for ($count = 0; $count < $numNodes; $count++) {
    $pathCount{$count} = 0;
    my $nextFile = $opt_d . "/" . $opt_p . "-" . $count;
    open NODEDIR, $nextFile || die "Cannot open $nextFile";
    my $nodeFile;
    my $initialTime = -1;
    while ($nodeFile = <NODEDIR>) {    
	if (index($nodeFile, "Recv") == -1) {
	    next;
	}
	chomp;
	my @line = split(/, /, $nodeFile);
	my $seconds = $line[1];
	my $src = $line[4];
	my $dst = $line[5];
	my $cost = $line[6];
      
	my @sizeline = split(/\[/, $line[2]);
	my $size = $sizeline[1];
	#print "$sizeline[1]\n";

	@line = split(/\]/, $line[2]);
	my $nanoseconds = $line[0] / 1000000000;
	my $time = ($seconds + $nanoseconds) / $opt_t;

	if ($initialTime == -1) {
	    $initialTime = $time;
	}
	my $timeLapsed = int($time - $initialTime);
	if ($totalBandwidth{$timeLapsed} == undef) {
	    $totalBandwidth{$timeLapsed} = $size;
	    $totalBandwidth1{$timeLapsed} = $size;
	} else {
	    $totalBandwidth{$timeLapsed} += $size;
	    $totalBandwidth1{$timeLapsed} += $size;
	}
	if ($timeLapsed > $maxTime) {
	    $maxTime = $timeLapsed; 
	}
    }

    if ($opt_r == "1") {
	my $outFile = $opt_d . "/" . $opt_p . ".out." . $count;
	open OUTFILE, $outFile || die "Cannot open $nextFile";
	my $lineStr;
	while ($lineStr = <OUTFILE>) {
	    if (index($lineStr, "PathsIn") == -1) {
		next;
	    }
	    #print $lineStr;
	    my @line = split(/, /, $lineStr);

	    my $seconds = $line[1];
	    my $nanoseconds = $line[2] / 1000000000;
	    my $time = ($seconds + $nanoseconds) / $opt_t;
	    my $timeLapsed = int($time - $initialTime);
	    my $savings = ($line[3] * $line[4]) - $line[5];
	    #print "$timeLapsed $savings $totalBandwidth1{$timeLapsed}\n";
	    $totalBandwidth1{$timeLapsed} = $totalBandwidth1{$timeLapsed} - $savings;
	}
    }

}

my $totalBandwidthUsage = 0;
my $totalBandwidthUsage1 = 0;
for ($count = 0; $count < $maxTime; $count++) {
    if ($totalBandwidth{$count} != undef) {
	my $time = $opt_t * $count;
	my $bw = $totalBandwidth{$count} / $opt_n / $opt_t / 1024;
	my $bw1 = $totalBandwidth1{$count} / $opt_n / $opt_t / 1024;
	print "Bandwidth usage: $time $bw\n";
	print BW "Bandwidth usage: $time $bw\n";
	if ($opt_r == "1") {
	    print BW1 "Bandwidth usage: $time $bw1\n";
	}
	$totalBandwidthUsage += $totalBandwidth{$count} / 1024 / 1024;
	$totalBandwidthUsage1 += $totalBandwidth1{$count} / 1024 / 1024;
    }
}
print "Total bandwidth: $totalBandwidthUsage MB\n";
print BW "Total bandwidth: $totalBandwidthUsage MB\n";
if ($opt_r == "1") {
    print BW1 "Total bandwidth: $totalBandwidthUsage1 MB\n";
}
close BW;
close BW1;
