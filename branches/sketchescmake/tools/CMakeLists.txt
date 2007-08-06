ADD_LIBRARY(p2tools
  src/tools/ExternalSort.cc
  src/tools/ExternalSort.h
  src/tools/SHA1.cc
  src/tools/SHA1.h
  src/tools/TemporaryFile.cc
  src/tools/Tools.cc
  include/Tools.h)

#TARGET_LINK_LIBRARIES(tools boost_regex boost_date_time crypto ssl)

INCLUDE_DIRECTORIES(src include)
