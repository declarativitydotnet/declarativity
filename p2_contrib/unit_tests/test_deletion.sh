#!/bin/bash

overlog=~/pieskovisko/p2/tests/runOverLog

xterm -geometry 150x25 -T port10000 -e $overlog -o test_delete.olg -p 10000 -n localhost &

xterm -geometry 150x25 -T port10001 -e $overlog -o test_delete.olg -p 10001 -n localhost &
