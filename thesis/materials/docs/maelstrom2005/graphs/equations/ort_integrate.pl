#! /usr/bin/perl

use strict;
my $alpha0=0.1;
my $key="ort";
if(@ARGV<3){
	print "Usage:$0 <f> <epoch> <key>\n";
	die;
}
my $f=$ARGV[0];
$key=$ARGV[2];
$alpha0=$f;
my $gamma=0.03;
my $beta=0.1;
my $nmin=16;
my $h=4;
my $g=16;
my $p1=0;
my $factor=0.1;
my $redundancy=1;
my $epoch=$ARGV[1];

my $max=10*$epoch;
my $dt=1;
my $alpha=$alpha0;
my $cutoff=0;
my $crt;
my $interval=10;
for(my $i=0;$i<$max;$i++){
	my $p=1-(1-$alpha)**$h;
	my $x=($p**$g)*(1-$alpha)+(1-$p**$g)*($f-$alpha);
	$x*=$gamma*$beta*$dt/$nmin;
	$alpha+=$x;
	my $ort=ort($i,$epoch,$crt);
	my $suc=1-((1-(1-$ort)**$h)*$redundancy);
	if(($i*$dt)%$epoch==0){
		$crt=$alpha;
	}
	if($i%$interval==0){
		my $j=$i/$dt;
		if($key eq "ort"){
			print $j."\t".$ort."\n";
		}elsif($key eq "success"){
			print $j."\t".$suc."\n";
		}
	}
}


sub ort{
	my $time=$_[0];
	my $epoch=$_[1];
	my $crt=$_[2];
	my $C=(1/(1-$crt)**($h+1))-1;
	my $t=$time%$epoch;
	my $exponent =$gamma*$beta*($h+1)*$t/$nmin;
	my $denom=(1+$C*exp($exponent))**(1/($h+1));
	my $alpha=1-1/$denom;
	return $alpha;
}
