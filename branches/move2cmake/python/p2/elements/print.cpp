#include <print.h>
#include <boost/python.hpp>
using namespace boost::python;

void export_print()
{
  class_<Print, bases<Element>, boost::shared_ptr<Print>, boost::noncopyable>
        ("Print", init<std::string>())
    .def("class_name", &Print::class_name)
    .def("processing", &Print::processing)
    .def("flow_code",  &Print::flow_code)
  ;
}
