ADD_LIBRARY(sketches
            src/CountMinFM.cc
            src/FM.cc
            include/Sketches.h)
            
ADD_DEPENDENCIES(sketches tools)
INCLUDE_DIRECTORIES(src include)
