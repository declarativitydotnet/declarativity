#include <loggerI.h>
#include <boost/python.hpp>

using namespace boost::python;

class LoggerIWrap : public LoggerI, public wrapper<LoggerI>
{
public:
  void log(std::string classname, std::string instancename,
           Reporting::Level severity, int errnum, std::string explanation) {
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
}
