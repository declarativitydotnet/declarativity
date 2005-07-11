#!/usr/bin/perl 

use strict 'vars';
use Getopt::Std;
use FindBin;

our($opt_f);
our($opt_n);

if ($#ARGV == -1){
    print "Usage buildCDF.pl [options]\n";
    print "options:\n";
    print "\t-f <inputFile> : Directory containing all the logs\n";
    exit 0;
}

if (!getopts('f:n:')){
    die "Unknown option specified\n";
}

if (!defined $opt_f) {
    die "Must have an input file name";
}

opendir (INDIR, "$opt_f") || die "Couldn't open $opt_f";

# go through each directory, look at the log file messages
# build an in-memory data structure on the lookups 
# 1) send <dst, lookup eventID, time>
# 2) receive <src, lookup eventID, time>
my $file;
my %receiveMsgs;
my %sendMsgs;
my %addresses;
my $numReceiveMsgs = 0;
my $numSendMsgs = 0;
while ($file = readdir(INDIR)) {
    if (index($file, "dat") == -1 &&
	index($file, "simple") == -1 &&
	index($file, "messages") == -1) {
	opendir (NODEDIR, $opt_f . "/" . $file) || die "Cannot open $file";
	my $nodeFile;
	while ($nodeFile = readdir(NODEDIR)) {
	    if (index($nodeFile, "out") != -1) {		
		open INFILE, $opt_f . "/" . $file . "/" . $nodeFile  
		    or die "Couldn't open $opt_f/$file/$nodeFile";
		my $nextLine;
		while ($nextLine = <INFILE>) {
		    chomp;
		    my @line = split(/, /, $nextLine);
		    if (index($nextLine, "ReceiveBefore") != -1 &&
			index($nextLine, "lookup,") != -1) {
			my $timeReceive = $line[1];
			my $destination = $line[3];
			my $eventID = substr($line[6], 0, length($line[6] - 2));
			#print $timeReceive . " " . $destination . " " . $eventID . "\n";
			@receiveMsgs{$destination . ":" . $eventID} = $timeReceive;
			@addresses{$destination} = $destination;
			$numReceiveMsgs += 1;
		    }

		    if (index($nextLine, "RemoteSend") != -1 &&
			index($nextLine, "lookup,") != -1) {
			my $timeReceive = $line[1];
			my $destination = $line[3];
			my $eventID = substr($line[6], 0, length($line[6] - 2));
			#print $timeReceive . " " . $destination . " " . $eventID . "\n";
			@sendMsgs{$destination . ":" . $eventID} = $timeReceive;
			$numSendMsgs += 1;
		    }
		}		
	    }
	}      	
    }
}

# count messages sent and latency
# percentage losses?
# latency, local timestamps
print "Finish processing $numReceiveMsgs $numSendMsgs\n";		

my $key;
my $failures = 0;
foreach $key (keys (%sendMsgs)) { 
    my @fields = split(/:/,  $key);    
    if (@addresses{"$fields[0]:$fields[1]"} != undef && 
	@receiveMsgs{$key} == undef) {
	$failures += 1;
    } else {
	my $latency = @sendMsgs{$key} - @receiveMsgs{$key};
    }
}
my $percentFailures = $failures / $numSendMsgs * 100;
print "Number of failures: $failures $percentFailures\n";

