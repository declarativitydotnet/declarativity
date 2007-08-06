ADD_LIBRARY(p2sketches
            src/CountMinFM.cc
            src/FM.cc
            include/Sketches.h)

TARGET_LINK_LIBRARIES(p2sketches p2tools)
     
ADD_DEPENDENCIES(p2sketches p2tools)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR} src include)
