#!/bin/bash
sudo rm p2.tar.gz
boostlibs=" regex* serialization* date_time* thread*"
boostfiles=${boostlibs// / libs/boost_1_35_0/build/lib/libboost_}
#boostfiles=${boostfiles//1/-gcc41-1_35.so}
#echo $boostfiles
tar cvfz p2.tar.gz p2/lang/olg `find release -name "*.so"` release/bin/runStagedOverlog $boostfiles

