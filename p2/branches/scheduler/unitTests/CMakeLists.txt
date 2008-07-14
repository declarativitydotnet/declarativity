ADD_EXECUTABLE(p2Test 				
				p2Test.C
				testTable2.C 
				testRefTable.C 
				testValues.C 
				testIDs.C 
				testFdbufs.C 
				testMarshal.C 
				testPel.C 
				testLists.C 
				testAggwrap.C)
#SET_TARGET_PROPERTIES(p2Test PROPERTIES HAS_CXX TRUE)
TARGET_LINK_LIBRARIES(p2Test ${COMMON_LIBRARIES} boost_unit_test_framework)

INCLUDE_DIRECTORIES(${P2_BINARY_DIR}/p2core)
