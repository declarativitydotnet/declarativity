#include <logger.h>
#include <boost/python.hpp>
using namespace boost::python;

void export_logger()
{
  class_<Logger, bases<Element, LoggerI>, boost::shared_ptr<Logger>, boost::noncopyable>
        ("Logger", init<std::string>())
    .def("class_name", &Logger::class_name)
    .def("processing", &Logger::processing)
    .def("flow_code",  &Logger::flow_code)
    
    /** Override this since it's pure virtual in the interface */
    .def("log", &Logger::log)
  ;
}
