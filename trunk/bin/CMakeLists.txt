ADD_EXECUTABLE(runOverLog runOverLog.C)
ADD_EXECUTABLE(runStagedOverlog runStagedOverlog.C)
ADD_EXECUTABLE(udpMessage udpMessage.C)
ADD_EXECUTABLE(udpListen udpListen.C)
ADD_EXECUTABLE(p2nc p2nc.C)

# need to explicitly mention dependencies on ol_lexer.h since CMake can't 
# figure it out.
ADD_DEPENDENCIES(runOverLog p2overlog)
ADD_DEPENDENCIES(runStagedOverlog p2overlog)

# Link libs for all these targets

TARGET_LINK_LIBRARIES(runOverLog ${COMMON_LIBRARIES})

TARGET_LINK_LIBRARIES(runStagedOverlog ${COMMON_LIBRARIES})

TARGET_LINK_LIBRARIES(udpMessage ${COMMON_LIBRARIES})

TARGET_LINK_LIBRARIES(udpListen ${COMMON_LIBRARIES})

TARGET_LINK_LIBRARIES(p2nc ${COMMON_LIBRARIES})

INCLUDE_DIRECTORIES(${P2_BINARY_DIR}/p2core)


INSTALL(TARGETS runStagedOverlog
            RUNTIME DESTINATION bin)
