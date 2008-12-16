#!/bin/bash
unison -fastcheck true -perms 0 -addversionno -auto $EXPROOT/distinf ssh://gs4436.sp.cs.cmu.edu//data/pokusy/distinf
