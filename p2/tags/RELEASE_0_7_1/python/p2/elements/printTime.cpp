#include <printTime.h>
#include <boost/python.hpp>
using namespace boost::python;

void export_printTime()
{
  class_<PrintTime, bases<Element>, boost::shared_ptr<PrintTime>, boost::noncopyable>
        ("PrintTime", init<std::string>())
    .def("class_name", &PrintTime::class_name)
    .def("processing", &PrintTime::processing)
    .def("flow_code",  &PrintTime::flow_code)
  ;
}
