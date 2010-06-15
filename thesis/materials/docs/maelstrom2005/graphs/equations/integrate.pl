#! /usr/bin/perl

use strict;
my $alpha0=0.1;
if(@ARGV<1){
	print "Usage:$0 <f (in %)> \n";
	die;
}
my $f=$ARGV[0]/100;
$alpha0=$f;
my $timestep="yes";
my $gamma=0.03;
my $beta=0.1;
my $nmin=16;
my $h=5;
my $g=16;
my $p1=0;
my $factor=0.1;

my $max=10*100000;
my $dt=1;
my $alpha=$alpha0;
my $cutoff=0;
my $interval=1000;
for(my $i=0;$i<$max;$i++){
	my $p=1-(1-$alpha)**$h;
	my $x=($p**$g)*(1-$alpha)+(1-$p**$g)*($f-$alpha);
	$x*=$gamma*$beta*$dt/$nmin;
	$alpha+=$x;
	if($timestep eq "yes" && $i%$interval==0){
		my $j=$i/1;
		print $j."\t".$alpha."\n";
	}
	if($alpha<=(1+$factor)*$f){
		$cutoff=$i;
	}
}
if($timestep eq "no"){
	print $cutoff*$dt."\n";
}
