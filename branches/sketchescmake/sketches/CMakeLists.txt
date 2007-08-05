ADD_LIBRARY(sketches
            src/CountMinFM.cc
            src/FM.cc
            include/Sketches.h)

TARGET_LINK_LIBRARIES(sketches tools boost_regex boost_date_time crypto ssl) 
     
ADD_DEPENDENCIES(sketches tools)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR} src include)
