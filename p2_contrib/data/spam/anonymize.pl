#!/usr/bin/perl -w
use Time::Local;
use Digest::SHA1  qw(sha1 sha1_hex sha1_base64);

sub to_unixtime {
    my $ts = shift;
    my ($foo, $bar) = split(' ', $ts);
    my ($year, $mon, $day) = split('-', $foo);
    my ($hr, $min, $sec) = split(':', $bar);
    $foo =  timegm($sec, $min, $hr, $day, $mon-1, $year);
    #print STDERR "$foo\n";
    return $foo;
}


my $dirname;
my $sha1Key;

if ($ARGV[0]) {
    $dirname = $ARGV[0];
} else {
    die "Need dir path where all data files reside";
}
if ($ARGV[1]) {
    $sha1Key = $ARGV[1];
} else {
    die "Need key for generating anonymized IP addresses";
}

opendir(DIR, $dirname) or die "cant open pipe $dirname: $!";
@files = grep(/.*\.lzo$/,readdir(DIR));
closedir(DIR);

my %fsm = ();
my %map = ();

foreach $filename (@files) {
    if ($filename =~ /maillog\.(\d{4})(\d{2})(\d{2})\.lzo/) {
        my $date = join('-', ($1, $2, $3));
        print STDERR "processing $filename\n";

        #open (PIPE, "lzop -dc $filename|") or die "cant open pipe $filename: $!";
        open (PIPE, "$filename") or die "cant open pipe $filename: $!";
        my $tempLog = "anon-maillog.$1$2$3.log";
        open (ANONYMIZED, ">$tempLog") or die "cant open pipe $tempLog: $!";
        while (<PIPE>) {
            chomp;
            if (/^.*(\d{2}:\d{2}:\d{2}).*: ([A-Z0-9]+): (.*)/) {
                my ($time, $code, $rest) = ($1, $2, $3);
                if ($rest =~ /client=.*\[(.*)\]/) {
                    $fsm{$code} = $1;
                } else {
                    if ($rest =~ /status=sent/ &&
                        defined($fsm{$code})) {
                        my $dom = "nil";
                        if ($rest =~ /orig_to=<(.*)>/ || $rest =~ /to=<(.*)>/) {
                            my @temp = split('@', $1);
                            $dom = $temp[1];
                        } else {
                            print STDERR "not matched: $rest\n";
                        }

                        my $IPKey = sha1_hex($fsm{$code}, $sha1Key);
                        $map{$fsm{$code}} = $IPKey;
                        my $unix_ts = to_unixtime("$date $time");

                        #print "Accept $unix_ts " . $fsm{$code} . " " . lc($dom) . "\n";
                        print ANONYMIZED "Accepted $date $time " . $IPKey . " " . lc($dom) . "\n";
                        undef $fsm{$code};
                    } elsif ($rest =~ /reject/) {
                        #print $rest;
                        if($rest =~ /\[(\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3})\]/) {
                            my $from = $1;
                            if($rest =~ /to=<(.*)>\s/) {
                                my @temp = split('@', $1);
                                $dom = $temp[1];
                            
                                my $IPKey = sha1_hex($from, $sha1Key);
                                $map{$from} = $IPKey;
                                my $unix_ts = to_unixtime("$date $time");

                                #print "Reject $unix_ts " . $fsm{$code} . " " . lc($dom) . "\n";
                                print ANONYMIZED "Rejected $date $time " . $IPKey . " " . lc($dom) . "\n";
                            }     
                        }
                        undef $fsm{$code};
                    
                    }
                }
            }
        }
        close (PIPE);
        close(ANONYMIZED);
    } else {
        die "unrecognized file format: $filename\n";
    }
}

#Storing the IP address and sha1 digest mapping
open(MAP, '>IPaddrMap.log');
print MAP "Key : $sha1Key \n";

while (($IPaddr, $digest) = each(%map))
{
	print MAP "$IPaddr " . "$digest\n";
}
close (MAP);