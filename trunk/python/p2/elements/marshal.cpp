#include <marshal.h>
#include <boost/python.hpp>
using namespace boost::python;

void export_marshal()
{
  class_<Marshal, bases<Element>, boost::shared_ptr<Marshal>, boost::noncopyable>
        ("Marshal", init<std::string>())
    .def("class_name", &Marshal::class_name)
    .def("processing", &Marshal::processing)
    .def("flow_code",  &Marshal::flow_code)
  ;
}
