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
TARGET_LINK_LIBRARIES(benchmark ${COMMON_LIBRARIES})

TARGET_LINK_LIBRARIES(runOverLog ${COMMON_LIBRARIES})

TARGET_LINK_LIBRARIES(runOverLogNoCPP ${COMMON_LIBRARIES})

TARGET_LINK_LIBRARIES(simple_lookup ${COMMON_LIBRARIES})

TARGET_LINK_LIBRARIES(lookupGenerator ${COMMON_LIBRARIES})

TARGET_LINK_LIBRARIES(udpMessage ${COMMON_LIBRARIES})

TARGET_LINK_LIBRARIES(udpListen ${COMMON_LIBRARIES})

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})