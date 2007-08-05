ADD_LIBRARY(sketches
            src/CountMinFM.cc
            src/FM.cc
            include/Sketches.h)
            
ADD_DEPENDENCIES(libsketches libtools)
INCLUDE_DIRECTORIES(src include)
