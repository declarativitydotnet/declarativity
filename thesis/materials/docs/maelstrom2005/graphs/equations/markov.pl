#! /usr/bin/perl

use strict;

if(@ARGV<2){
	die "Usage:$0 n s\n";
}

my $n=$ARGV[0];
my $s=int($ARGV[1]+0.5);
my $max=100;

my @matrix;
for(my $j=0;$j<=$n;$j++){
	$matrix[0][$j]=0;
}
$matrix[0][0]=1;

for(my $i=1;$i<=$max;$i++){
	$matrix[$i][0]=$matrix[$i-1][0]*0;
	for(my $j=1;$j<=$n;$j++){
		my $sum=0;
		for(my $k=0;$k<=$j;$k++){
			$sum+=C($k,$s-$j+$k)*C($n-$k,$j-$k)*$matrix[$i-1][$k]/C($n,$s);
		}
		if($j==$n){
			$sum-=$matrix[$i-1][$j];
		}
		$matrix[$i][$j]=$sum;
	}
#	print "\nI=".$i."\n";
#	for(my $j=0;$j<=$n;$j++){
#		print $matrix[$i][$j]."\t";
#	}
	
}
my $E=0;
for(my $i=0;$i<=$max;$i++){
	$E+=$i*$matrix[$i][$n];
}

print $E."\n";

sub C{
	my $n=$_[0];
	my $k=$_[1];
	if($k>$n || $k<0){
		return 0;
	}
	return fact($n)/(fact($k)*fact($n-$k));
}

sub fact{
	my $n=$_[0];
	my $fact=1;
	for(my $i=$n;$i>0;$i--){
		$fact*=$i;
	}
	if($n<0){
		$fact=0;
	}
	return $fact;
}
