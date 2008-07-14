#include <loggerI.h>
#include <boost/python.hpp>

using namespace boost::python;

class LoggerIWrap : public LoggerI, public wrapper<LoggerI>
{
public:
  void log(string classname, string instancename,
           Level severity, int errnum, string explanation) {
    this->get_override("log")(classname, instancename, severity, 
                              errnum, explanation);
  };
};

void export_loggerI()
{
  scope outer = 
    class_<LoggerIWrap, boost::noncopyable>("LoggerI", no_init)
       .def("log", &LoggerIWrap::log)
    ;

    enum_<LoggerIWrap::Level>("Level")
      .value("ALL",   LoggerI::ALL)
      .value("WORDY", LoggerI::WORDY)
      .value("INFO",  LoggerI::INFO)
      .value("WARN",  LoggerI::WARN)
      .value("ERROR", LoggerI::ERROR)
      .value("NONE",  LoggerI::NONE)
    ;
}
