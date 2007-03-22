ADD_EXECUTABLE(benchmark benchmark.C)
ADD_EXECUTABLE(runOverLog runOverLog.C)
#SET_TARGET_PROPERTIES(runOverLog PROPERTIES HAS_CXX TRUE)
ADD_EXECUTABLE(runOverLogNoCPP runOverLogNoCPP.C)
#SET_TARGET_PROPERTIES(runOverLogNoCPP PROPERTIES HAS_CXX TRUE)
ADD_EXECUTABLE(simple_lookup simple_lookup.C)
#SET_TARGET_PROPERTIES(simple_lookup PROPERTIES HAS_CXX TRUE)
ADD_EXECUTABLE(lookupGenerator lookupGenerator.C)
#SET_TARGET_PROPERTIES(lookupGenerator PROPERTIES HAS_CXX TRUE)
ADD_EXECUTABLE(udpMessage udpMessage.C)
#SET_TARGET_PROPERTIES(udpMessage PROPERTIES HAS_CXX TRUE)
ADD_EXECUTABLE(udpListen udpListen.C)
#SET_TARGET_PROPERTIES(udpListen PROPERTIES HAS_CXX TRUE)

# Link libs for all these targets
TARGET_LINK_LIBRARIES(benchmark boost_regex boost_unit_test_framework boost_date_time ssl crypto p2planner p2overlog p2net p2stages p2elements p2debugger p2core p2eventLoop stdc++)

TARGET_LINK_LIBRARIES(runOverLog boost_regex boost_unit_test_framework boost_date_time ssl crypto p2planner p2overlog p2net p2stages p2elements p2debugger p2core p2eventLoop stdc++)

TARGET_LINK_LIBRARIES(runOverLogNoCPP boost_regex boost_unit_test_framework boost_date_time ssl crypto p2planner p2overlog p2net p2stages p2elements p2debugger p2core p2eventLoop stdc++)

TARGET_LINK_LIBRARIES(simple_lookup boost_regex boost_unit_test_framework boost_date_time ssl crypto p2planner p2overlog p2net p2stages p2elements p2debugger p2core p2eventLoop stdc++)

TARGET_LINK_LIBRARIES(lookupGenerator boost_regex boost_unit_test_framework boost_date_time ssl crypto p2planner p2overlog p2net p2stages p2elements p2debugger p2core p2eventLoop stdc++)

TARGET_LINK_LIBRARIES(udpMessage boost_regex boost_unit_test_framework boost_date_time ssl crypto p2planner p2overlog p2net p2stages p2elements p2debugger p2core p2eventLoop stdc++)

TARGET_LINK_LIBRARIES(udpListen boost_regex boost_unit_test_framework boost_date_time ssl crypto p2planner p2overlog p2net p2stages p2elements p2debugger p2core p2eventLoop stdc++)
