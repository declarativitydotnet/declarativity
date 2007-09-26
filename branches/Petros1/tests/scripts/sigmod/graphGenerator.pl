#!/usr/bin/perl 
# Create a Chord network exponentially, with degree d.  First gateway,
# then d nodes off of that, then d^2 nodes off of those, etc.

# Once it is all set up, start running random lookups from random nodes

use strict 'vars';
use Getopt::Std;
use FindBin;

our($opt_e);
our($opt_n);
our($opt_t);
our($opt_d);
#our($opt_c);
our($opt_a);

if ($#ARGV == -1){
    print "Usage graphGenerator.pl [options]\n";
    print "options:\n";
    print "\t-n <numNodes> : Number of nodes\n";
    print "\t-e <edges> : Number of edges\n";
    print "\t-t <type> : 0 - ring, 1 - random, others - gtitm\n";
    print "\t-d <type> : 0 - undirected, 1 - directed\n";
    #print "\t-c <type> : 0 - hop, 1 - random\n";
    print "\t-a <file>: 0 - no input address file\n";
    exit 0;
}

if (!getopts('n:e:t:d:a:')){
    die "Unknown option specified\n";
}

if (!defined $opt_n) {
    die "Must have number of nodes\n";
}

my $numNodes = $opt_n;

if (!defined $opt_d) {
    die "Must be directed or not";
}

if (!defined $opt_t) {
    die "Must have network type";
}

if ($opt_t != "0" && !defined $opt_e) {
    die "Number of edges unknown";
}

#if (!defined $opt_c) {
#    die "Must have cost metric type";
#}

if (!defined $opt_a) {
    die "Must have address field file";
}
	

my $directed = $opt_d;
my $type = $opt_t;

if ($type eq "0") {
    my $count = 0;
    for ($count = 0; $count < $numNodes; $count++) {
	my $dst = ($count + 1) % $numNodes;
	my $src = $count % $numNodes;
	print "$src $dst 1\n";
	if ($directed == "0") {
	    print "$dst $src 1\n";
	}
    }
}

srand(rand(1000));

# read the address file if it is present
my %addr;
my $addrFileName = $opt_a;
my %precomputeLatencies;
my $precompute = false;
if (index($type, "precompute") != -1) {
    $precompute = true;
    my $addrFile;
    open ADDRFILE, $type || die "Cannot open $opt_a";
    while ($addrFile = <ADDRFILE>) {    
	chop($addrFile);
	my @line = split(/, /, $addrFile);
	@precomputeLatencies{$line[0] . "." . $line[1]} = $line[2];
	#print "$line[0] $line[1] $line[2]\n";
    }
}


if ($addrFileName ne "0") {
    #print "$addrFileName\n";
    open ADDRFILE, $addrFileName || die "Cannot open $opt_a";
    #print "$opt_a\n";
    my $addrFile;
    my $initialTime = -1;
    my $count = 0;
    while ($addrFile = <ADDRFILE>) {    
	chop($addrFile);
	$addr{$count} = $addrFile;
	#print "$count $addr{$count}\n";
	$count ++;
    }
}


my %edgesSoFar;
my %degree;
my $edgeIncr;
if ($directed == "0") {
    $edgeIncr = 2;
} else {
    $edgeIncr = 1;
}
open NS, ">precompute\_$opt_n.NS" || die "Cannot open $precompute\_$opt_n";
my $linkCount = 0;
if ($type eq "1" || 
    index($type, "precompute") != -1) {
    my $edges = $opt_e * $numNodes;
    my $edgeCount = 0;
    while ($edgeCount < $edges) {
	#random src, dst
	my $src = int(rand($numNodes));
	my $dst = int(rand($numNodes));
	my $searchStr = $src.".".$dst;

	my $srcStr;
	my $dstStr;
	if ($opt_a ne "0") {
	    $srcStr = @addr{$src};
	    $dstStr = @addr{$dst};
	    #print "Test $src $dst $srcStr $dstStr\n";
	} else {
	    $srcStr = $src;
	    $dstStr = $dst;
	}

	# if present, proceed
	if ($src != $dst &&
	    @edgesSoFar{$searchStr} == undef) {
	    
	    if (@degree{$src} >= (1*$edgeIncr*$opt_e) ||
		@degree{$dst} >= (1*$edgeIncr*$opt_e)) {
		#print "Skip $src $dst\n";
		next;
	    }

	    my $cost = 1;

	    if ($precompute eq false) {
		print "$srcStr $dstStr 1 " . (rand(10)+1) . " 1\n";		
	    } else {
		my $lat = $precomputeLatencies{$searchStr};
		my $srcStrStr = $srcStr;
		my $srcT = 4 - length $srcStr;
		my $dstT = 4 - length $dstStr;
		my $dstStrStr = $dstStr;
		my $i;
		for ($i = 0; $i < $srcT; $i++) {
		    $srcStrStr = "0".$srcStrStr;
		}
		for ($i = 0; $i < $dstT; $i++) {
		    $dstStrStr = "0".$dstStrStr;
		}
		print "$srcStr $dstStr 1 $lat\n";			
		print NS "set link$linkCount [ns duplex-link \$node$srcStrStr \$node$dstStrStr 10Mb $lat"."ms DropTail]\n";
		$linkCount++;
	    }

	    @edgesSoFar{$searchStr} = $searchStr;
	    if ($directed == "0") {
		if ($precompute eq false) {
		    print "$dstStr $srcStr 1 " . (rand(10)+1) . " " . (rand(5)+1) . "\n";		
		} else {
		    print "$dstStr $srcStr 1 $precomputeLatencies{$searchStr}\n";		
		}
		$searchStr = $dst.".".$src;
		@edgesSoFar{$searchStr} = $searchStr;
	    }
	    $edgeCount += $edgeIncr;

	    if (@degree{$src} == undef) {
		@degree{$src} = $edgeIncr;
	    } else {
		@degree{$src} += $edgeIncr;
	    }

	    if (@degree{$dst} == undef) {
		@degree{$dst} = $edgeIncr;
	    } else {
		@degree{$dst} += $edgeIncr;
	    }
	}
    }
}
print NS "tb-use-endnodeshaping   1\n";
close NS;

#GT-ITM file
my $addrFile;
if (!($type eq "0" || $type eq "1")) {
    if (index($type, "precompute") == -1) {
	open ADDRFILE, $type || die "Cannot open $opt_a";
	while ($addrFile = <ADDRFILE>) {    
	    chop($addrFile);
	    my @line = split(/ /, $addrFile);
	    my $rand = rand(10) + 1;
	    print "@addr{$line[0]} @addr{$line[1]} $line[2] $line[3] $line[4] $rand\n";
	}
    }
}

#my $addrFile;
#if (!($type eq "0" || $type eq "1")) {
#    }
#}



my $count = 0;
for ($count = 0; $count < $numNodes; $count++)
{
    #print "$count @degree{$count}\n";
}






