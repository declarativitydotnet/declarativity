SUBDIRS(unitTests)

ADD_EXECUTABLE(benchmark benchmark.C)

# need to explicitly mention dependencies on ol_lexer.h since CMake can't 
# figure it out.

# Link libs for all these targets

TARGET_LINK_LIBRARIES(benchmark ${COMMON_LIBRARIES})

INCLUDE_DIRECTORIES(${P2_BINARY_DIR}/p2core)

