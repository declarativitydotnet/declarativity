#!/usr/bin/perl 
# Create a Chord network exponentially, with degree d.  First gateway,
# then d nodes off of that, then d^2 nodes off of those, etc.

# Once it is all set up, start running random lookups from random nodes

use strict 'vars';
use Getopt::Std;
use FindBin;

our($opt_s, $opt_n, $opt_d, $opt_N, $opt_l, $opt_D);

if ($#ARGV == -1){
    print "Usage runLocalHostChords.pl [options]\n";
    print "options:\n";
    print "\t-n <degree>    : Multiple each wave by this much\n";
    print "\t-d <delay>     : Delay between waves by this many seconds (float)\n";
    print "\t-D <startDelay>: Delay between issuance and join by this many seconds (flot)\n";
    print "\t-s <num>       : Random seed\n";
    print "\t-N <nodes>     : Total number of nodes to run\n";
    print "\t-l <logging>   : Logging level\n";
    exit 0;
}

if (!getopts('n:d:s:N:l:D:')){
    die "Unknown option specified\n";
}

if (!defined $opt_s) {
    $opt_s = 0;
}

if (!defined $opt_n) {
    $opt_n = 1;
}

if (!defined $opt_N) {
    $opt_N = 2;
}

if (!defined $opt_d) {
    $opt_d = 2.0;
} else {
    $opt_d = $opt_d + 0.0;
}

if (!defined $opt_D) {
    $opt_D = 1.0;
} else {
    $opt_D = $opt_D + 0.0;
}

if (!defined $opt_l) {
    $opt_l = "NONE";
}

# Start the first gateway
my $firstPort = 10000;
my $localHost = "127.0.0.1";
my $firstGateway = "$localHost:$firstPort";
my $firstLog = "$firstGateway.log";
my $nextSeed = 0;
my $firstLine = "tests/runChord 0 $opt_l $nextSeed $firstGateway $opt_D >& $firstLog &";
print "$firstLine\n";
system($firstLine);
$nextSeed += 100;

my $remainingNodes = $opt_N - 1;
print "My remaining nodes are $remainingNodes\n";
my $level = 1;
my $levelGateways = 1;
my $firstLevelGateway = 10000;
my $nextNode = 10001;
my $levelNodes = $opt_n;

while($remainingNodes > 0) {
    #Create new level
    my $slept = sleep($opt_d);
    print "After sleeping $slept secs, Level $level\n";
    $levelNodes = $levelGateways * $opt_n;
    if ($levelNodes > $remainingNodes) {
	$levelNodes = $remainingNodes;
    }
    $remainingNodes -= $levelNodes;
    my $nextFirstGateway = $nextNode;

    my $currentGateway = 0;
    while ($levelNodes > 0) {
	my $nextGateway = $firstLevelGateway + ($currentGateway % $levelGateways);
	my $nextGatewayAddress = "$localHost:$nextGateway";
	my $nextNodeAddress = "$localHost:$nextNode";
	my $nextLog = "$nextNodeAddress.log";
	my $nextLine = "tests/runChord 0 $opt_l $nextSeed $nextNodeAddress $opt_D $nextGatewayAddress >& $nextLog&";
	print "$nextLine\n";
	system($nextLine);
	$currentGateway++;
	$nextNode++;
	$levelNodes--;
	$nextSeed += 100;
    }
    $level++;
    $levelGateways = $levelGateways * $opt_n;
    $firstLevelGateway = $nextFirstGateway;
}




#End
