#!/bin/bash

#distinf=/proj/P2/distinf

distinf=/p2/distinf
# the build will appear to be located in the local directory on the target
# machine. in particular, the shared libraries will be loaded locally

echo Update and build prl
cd $distinf/prl
svn update
#./build_project.sh

echo Update and build p2
cd $distinf/p2
svn update
cd $distinf/p2/p2core/pel
./pel_gen.py
cd $distinf/release
make

echo Running p2 to compile the dataflows
$OVERLOG -o $distinf/distinf/unit_tests/quit.olg

echo Make p2.tar.gz 
cd $distinf
./tar_p2.sh
