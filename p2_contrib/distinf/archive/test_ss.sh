#!/bin/bash
cp test_ss.olg 1.test_ss.olg
~/workspace/distinf/runOverLog -o 1.test_ss.olg -DMYID=1 -p 10001 2> test1.log &
cp test_ss.olg 2.test_ss.olg
~/workspace/distinf/runOverLog -o 2.test_ss.olg -DMYID=2 -p 10002 2> test2.log &
cp test_ss.olg 3.test_ss.olg
~/workspace/distinf/runOverLog -o 3.test_ss.olg -DMYID=3 -p 10003 2> test3.log &
cp test_ss.olg 4.test_ss.olg
~/workspace/distinf/runOverLog -o 4.test_ss.olg -DMYID=4 -p 10004 2> test4.log &

