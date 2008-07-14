ADD_EXECUTABLE(p2Test 				
				p2Test.C
				testTable2.C 
				testRefTable.C 
#				testBasicElementPlumbing.C 
				testValues.C 
				testIDs.C 
				testFdbufs.C 
#				testCsv.C 
				testMarshal.C 
				testPel.C 
				testLists.C 
				testAggwrap.C)
#SET_TARGET_PROPERTIES(p2Test PROPERTIES HAS_CXX TRUE)
TARGET_LINK_LIBRARIES(p2Test ${COMMON_LIBRARIES})

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR} ${P2_SOURCE_DIR}/elements ${P2_SOURCE_DIR}/stages ${P2_SOURCE_DIR}/getopt)
