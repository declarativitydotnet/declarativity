#!/bin/bash
cp jt_bug.olg .1.olg
cp jt_bug.olg .2.olg
cp jt_bug.olg .3.olg

xterm -geometry 180x25+30+30 -T 1 -e runp2 -p 10001 -o .1.olg &
xterm -geometry 180x25+60+60 -T 2 -e runp2 -p 10002 -o .2.olg &
xterm -geometry 180x25+90+90 -T 3 -e runp2 -p 10003 -o .3.olg &

echo "Press Enter to kill P2"
read

../tests/killp2
