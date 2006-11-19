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
our($opt_q);

if ($#ARGV == -1){
    print "Usage crunchResults.pl [options]\n";
    print "options:\n";
    print "\t-n <numNodes> : Number of nodes\n";
    print "\t-p <prefix> : log prefix\n";
    print "\t-d <dir> : Directory\n";
    print "\t-q <individual queries>: 0 - false\n";
    exit 0;
}

if (!getopts('n:p:d:t:a:r:q:')){
    die "Unknown option specified\n";
}

my $count;
my %queryBytes;;
my %savingsBytes;;
my $totalBytes = 0;
open BW, ">$opt_d/$opt_p-bw" || die "Cannot open $opt_d/$opt_p-bw";
open BW1, ">$opt_d/$opt_p-totalbw" || die "Cannot open $opt_d/$opt_p-totalbw";

for ($count = 0; $count < $opt_n; $count++) {
    my $outFile = $opt_d . "/" . $opt_p . ".out." . $count;

    open OUTDIR, $outFile || die "Cannot open $outFile";
    my $str;
    while ($str = <OUTDIR>) {    
	if (index($str, "Savings") == -1) {
	    next;
	}
	my @line = split(/ /, $str);
	if ($savingsBytes{$line[2]} == undef) {
	    $savingsBytes{$line[2]} = $line[3]; 
	} else {
	    $savingsBytes{$line[2]} += $line[3]; 
	}
	#print "$outFile $line[2] $line[3]\n";
    }
    close OUTDIR;

    my $nextFile = $opt_d . "/" . $opt_p . "-" . $count;
    open NODEDIR, $nextFile || die "Cannot open $nextFile";
    my $nodeFile;
    while ($nodeFile = <NODEDIR>) {    
	if (index($nodeFile, "Send") == -1) {
	    next;
	}
	#print $nodeFile . "\n";
	if (index($nodeFile, "bestPathReverse") != -1) {
	    #next;
	}
	chomp;
	my @line = split(/, /, $nodeFile);
	my $seconds = $line[1];
	my $src = $line[4];
	my $dst = $line[5];
	my $cost = $line[6];
      
	my @sizeline = split(/\[/, $line[2]);
	my $size = $sizeline[1];
	if ($opt_q eq 1) {
	    $size -= 12;
	}
	$totalBytes += $size;
	#print $size . "\n";
	
	if ($opt_q eq 1) {
	    my $index = index($nodeFile, "]");
	    $index = index($nodeFile, "]", $index+1);
	    my $substr = substr($nodeFile, $index-10,9);
	    my @queryLine = split(/, /, $substr);
	    if ($queryBytes{$queryLine[1]} == undef) {
		$queryBytes{$queryLine[1]} = $size / (1024 * 1024); 
	    } else {
		$queryBytes{$queryLine[1]} += $size / (1024 * 1024); 
	    }

	}
    }
}
$totalBytes = $totalBytes / (1024 * 1024);
my @keys = keys(%queryBytes);
my $keySize = @keys;
my $cum = 0;
my $count1;
for ($count = 1; $count <= $keySize; $count++) {
    if ($queryBytes{$count} == undef) {
	$queryBytes{$count} = 0;
    }
    if ($savingsBytes{$count} == undef) {
	$savingsBytes{$count} = 0;
    }
    my $savings = $savingsBytes{$count} / (1024 * 1024);
    my $usage = $queryBytes{$count} - $savings;
    if ($usage < 0) { $usage = 0; }
    #if ($usage > 1) { 
	$cum += $usage;
	print BW "$count $queryBytes{$count} $savings $usage $cum\n";
	$count1++;
    #}
}
if ($opt_q eq 1) {
    print BW1 "$cum\n";
} else {
    print BW1 "$totalBytes\n";
}
close BW;

system("cat $opt_d/$opt_p-bw");
system("cat $opt_d/$opt_p-totalbw");
close(BW1)
