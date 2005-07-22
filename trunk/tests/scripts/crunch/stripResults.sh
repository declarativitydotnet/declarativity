grep "Results" $1/*/*.out | grep "Receive" | grep "simple" > $1/simple_results
grep "lookup" $1/*/lookups.log > $1/allLookups.out

