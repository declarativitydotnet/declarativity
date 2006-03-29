#include <skr.h>
#include <boost/python.hpp>

using namespace boost::python;

void export_skr()
{
  class_<SimpleKeyRouter, bases<Element>, boost::shared_ptr<SimpleKeyRouter>, boost::noncopyable>
        ("SimpleKeyRouter", init<string, ValuePtr, optional<bool> >())
    .def("class_name", &SimpleKeyRouter::class_name)
    .def("processing", &SimpleKeyRouter::processing)
    .def("flow_code",  &SimpleKeyRouter::flow_code)
  ;
}
