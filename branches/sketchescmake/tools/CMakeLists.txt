ADD_LIBRARY(tools
  src/tools/ExternalSort.cc
  src/tools/ExternalSort.h
  src/tools/SHA1.cc
  src/tools/SHA1.h
  src/tools/TemporaryFile.cc
  src/tools/Tools.cc
  include/Tools.h)
  
INCLUDE_DIRECTORIES(src include)