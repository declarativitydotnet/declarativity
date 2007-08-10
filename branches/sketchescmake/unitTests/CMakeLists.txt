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
               testAggwrap.C
               testSketches.C)

#SET_TARGET_PROPERTIES(p2Test PROPERTIES HAS_CXX TRUE)

TARGET_LINK_LIBRARIES(p2Test boost_regex boost_serialization p2sketches p2tools boost_unit_test_framework boost_date_time ssl crypto p2overlog p2net p2stages p2elements p2debugger p2core p2eventLoop stdc++)

INCLUDE_DIRECTORIES(${P2_SOURCE_DIR}/elements 
                    ${P2_SOURCE_DIR}/stages
                    ${P2_SOURCE_DIR}/sketches/include
                    ${P2_SOURCE_DIR}/tools/include
)
