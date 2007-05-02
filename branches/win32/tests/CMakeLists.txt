ADD_EXECUTABLE(benchmark benchmark.C)
ADD_EXECUTABLE(runOverLog runOverLog.C)
ADD_EXECUTABLE(runOverLogNoCPP runOverLogNoCPP.C)
ADD_EXECUTABLE(simple_lookup simple_lookup.C)
ADD_EXECUTABLE(lookupGenerator lookupGenerator.C)
ADD_EXECUTABLE(udpMessage udpMessage.C)
ADD_EXECUTABLE(udpListen udpListen.C)

# need to explicitly mention dependencies on ol_lexer.h since CMake can't 
# figure it out.
ADD_DEPENDENCIES(runOverLog p2overlog)
ADD_DEPENDENCIES(runOverLogNoCPP p2overlog)

# Link libs for all these targets
TARGET_LINK_LIBRARIES(benchmark libeay32MDd p2planner p2overlog p2net p2stages p2elements p2debugger p2core getopt)

TARGET_LINK_LIBRARIES(runOverLog libeay32MDd p2planner p2overlog p2net p2stages p2elements p2debugger p2core getopt)

TARGET_LINK_LIBRARIES(runOverLogNoCPP libeay32MDd p2planner p2overlog p2net p2stages p2elements p2debugger p2core getopt)

TARGET_LINK_LIBRARIES(simple_lookup libeay32MDd p2planner p2overlog p2net p2stages p2elements p2debugger p2core getopt)

TARGET_LINK_LIBRARIES(lookupGenerator libeay32MDd p2planner p2overlog p2net p2stages p2elements p2debugger p2core getopt)

TARGET_LINK_LIBRARIES(udpMessage libeay32MDd p2planner p2overlog p2net p2stages p2elements p2debugger p2core getopt)

TARGET_LINK_LIBRARIES(udpListen libeay32MDd p2planner p2overlog p2net p2stages p2elements p2debugger p2core getopt)
