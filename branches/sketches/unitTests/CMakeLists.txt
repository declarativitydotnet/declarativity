ADD_EXECUTABLE(p2Test 				
				p2Test.C
				testTable2.C 
				testRefTable.C 
				testBasicElementPlumbing.C 
				testValues.C 
				testIDs.C 
				testFdbufs.C 
				testCsv.C 
				testMarshal.C 
				testPel.C 
				testLists.C 
				testAggwrap.C)
#SET_TARGET_PROPERTIES(p2Test PROPERTIES HAS_CXX TRUE)
TARGET_LINK_LIBRARIES(p2Test libboost_unit_test_framework-vc80-mt-gd-1_33_1 libeay32MDd p2overlog p2net p2stages p2elements p2debugger p2core getopt)

INCLUDE_DIRECTORIES(${P2_SOURCE_DIR}/elements ${P2_SOURCE_DIR}/stages ${P2_SOURCE_DIR}/getopt)
