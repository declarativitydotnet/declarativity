/*
* Copyright (c) 2003 Intel Corporation
* All rights reserved.
*
*/

#include "../p2_python.h"
#include "plumber.h"
#include "elementSpec.h"
#include "loggerI.h"

class LoggerIWrap : public LoggerI, public wrapper<LoggerI>
{
public:
  void log(string classname, string instancename,
           Level severity, int errnum, string explanation) {
    this->get_override("log")(classname, instancename, severity, 
                              errnum, explanation);
  };
};

BOOST_PYTHON_MODULE(p2core)
{
  #include "plumber.cpp"
  #include "elementSpec.cpp"
  #include "loggerI.cpp"
}
