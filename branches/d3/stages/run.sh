#!/bin/bash

echo $1
cp ../logrepo_1123_comp/xtrace_r$1.millennium.berkeley.edu xtrace.log
cp ../logrepo_1123_comp/ganglia_r$1.millennium.berkeley.edu ganglia.log

#cp ../logrepo/xtrace_r$1.millennium.berkeley.edu xtrace.log
#cp ../logrepo/ganglia_r$1.millennium.berkeley.edu ganglia.log

../tests/runOverLog -o remote.olg
