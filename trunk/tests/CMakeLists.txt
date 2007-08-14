ADD_EXECUTABLE(benchmark benchmark.C)
ADD_EXECUTABLE(runOverLog runOverLog.C)
ADD_EXECUTABLE(runStagedOverLog runStagedOverLog.C)
ADD_EXECUTABLE(udpMessage udpMessage.C)
ADD_EXECUTABLE(udpListen udpListen.C)
ADD_EXECUTABLE(p2nc p2nc.C)

# need to explicitly mention dependencies on ol_lexer.h since CMake can't 
# figure it out.
ADD_DEPENDENCIES(runOverLog p2overlog)
ADD_DEPENDENCIES(runStagedOverLog p2overlog)

# Link libs for all these targets

TARGET_LINK_LIBRARIES(benchmark ${COMMON_LIBRARIES})

TARGET_LINK_LIBRARIES(benchmark ${COMMON_LIBRARIES})

TARGET_LINK_LIBRARIES(runOverLog ${COMMON_LIBRARIES})

TARGET_LINK_LIBRARIES(runStagedOverLog ${COMMON_LIBRARIES}  p2cpp p2dl)

TARGET_LINK_LIBRARIES(udpMessage ${COMMON_LIBRARIES})

TARGET_LINK_LIBRARIES(udpListen ${COMMON_LIBRARIES})

TARGET_LINK_LIBRARIES(p2nc ${COMMON_LIBRARIES})

INCLUDE_DIRECTORIES(${P2_BINARY_DIR}/p2core)


INSTALL(TARGETS runOverLog runStagedOverLog udpMessage udpListen
            RUNTIME DESTINATION bin
            LIBRARY DESTINATION lib
            ARCHIVE DESTINATION lib)
