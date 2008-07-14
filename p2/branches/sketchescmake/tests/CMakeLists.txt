ADD_EXECUTABLE(benchmark benchmark.C)
ADD_EXECUTABLE(runOverLog runOverLog.C)
ADD_EXECUTABLE(runOverLogNoCPP runOverLogNoCPP.C)
ADD_EXECUTABLE(simple_lookup simple_lookup.C)
ADD_EXECUTABLE(lookupGenerator lookupGenerator.C)
ADD_EXECUTABLE(udpMessage udpMessage.C)
ADD_EXECUTABLE(udpListen udpListen.C)

# need to explicitly mention dependencies on ol_lexer.h since CMake can't 
# figure it out.
ADD_DEPENDENCIES(runOverLog overlog)
ADD_DEPENDENCIES(runOverLogNoCPP overlog)

# Link libs for all these targets
TARGET_LINK_LIBRARIES(benchmark boost_regex boost_date_time ssl crypto p2planner p2overlog p2net p2stages p2elements p2debugger p2core p2eventLoop stdc++)

TARGET_LINK_LIBRARIES(runOverLog boost_regex boost_date_time ssl crypto p2planner p2overlog p2net p2stages p2elements p2debugger p2core p2eventLoop stdc++)

TARGET_LINK_LIBRARIES(runOverLogNoCPP boost_regex boost_date_time ssl crypto p2planner p2overlog p2net p2stages p2elements p2debugger p2core p2eventLoop stdc++)

TARGET_LINK_LIBRARIES(simple_lookup boost_regex boost_date_time ssl crypto p2planner p2overlog p2net p2stages p2elements p2debugger p2core p2eventLoop stdc++)

TARGET_LINK_LIBRARIES(lookupGenerator boost_regex boost_date_time ssl crypto p2planner p2overlog p2net p2stages p2elements p2debugger p2core p2eventLoop stdc++)

TARGET_LINK_LIBRARIES(udpMessage boost_regex boost_date_time ssl crypto p2planner p2overlog p2net p2stages p2elements p2debugger p2core p2eventLoop stdc++)

TARGET_LINK_LIBRARIES(udpListen boost_regex boost_date_time ssl crypto p2planner p2overlog p2net p2stages p2elements p2debugger p2core p2eventLoop stdc++)
