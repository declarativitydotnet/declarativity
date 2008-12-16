#!/usr/bin/perl -w

# **********************************************************
# Outputs domain map listing domains in descending order of 
# the frequency of emails received
#       column 1: frequency of emails sent to the domain
#       column 2: domain number 
#       column 3: domain name 
# **********************************************************
        
use strict;
use POSIX;
use POSIX qw(strftime);
use Getopt::Std;
use Time::Local;

my $minargs = 3;
my $maxargs = 3;
my $argusage = "<anonymized-dir> <domain-map> <frequency-map>";
sub usage {
    die "Usage: $0 $argusage\n" .
        "";
}


my %flags = ();
getopts("", \%flags);
usage() if (@ARGV < $minargs or @ARGV > $maxargs);

my $anonDir = shift;
my $domains = shift;
my $f = shift;

opendir(DH, $anonDir) or die "Couldn't open $anonDir for reading: \n";

my @files = ();
my @dirFiles = readdir(DH);
foreach my $file(@dirFiles) 
{
    if (!(($file eq ".") || ($file eq "..") || ($file eq ".DS_Store")))
    {
        my $filename = "$anonDir/$file";
        # print "$filename \n";
        push(@files, $filename) unless -d $filename;
    }
}


my %domainMap = ();;
my %frequencyMap = ();
my %num = ();
my $numDom = 0;
my $name = "";

foreach $name (@files)
{
    print "Parsing $name \n";
    open(ANON, "$name") or die("Cannot read file: $name");
    while(<ANON>){
        
        my @vecWords = split(/\s+/, $_);
        next unless ($#vecWords== 4);
        my $domainValue = $vecWords[4];
        if (!exists($domainMap{$domainValue})){
            $numDom++;
            $domainMap{$domainValue} = "$numDom";
            $num{$numDom} = "$domainValue";
            $frequencyMap{$numDom} = "1";
        }
        else{
            my $index = $domainMap{$domainValue};
            $frequencyMap{$index} = $frequencyMap{$index} + 1;
        }
    }
    close ANON;
}


open(DOM, ">$domains") or die("Cannot read file: $domains");
foreach my $key (sort {$domainMap{$a} <=> $domainMap{$b}} keys %domainMap){ 
     print DOM "$domainMap{$key} $key\n";
}
close DOM;            


open(FMAP, ">$f") or die("Cannot read file: $f");
foreach my $key (sort {$frequencyMap{$b} <=> $frequencyMap{$a}} keys %frequencyMap){ 
     print FMAP "$frequencyMap{$key} $key $num{$key}\n";
}
close FMAP;

print "Total domains: $numDom \n";