#include <traceTuple.h>
#include <boost/python.hpp>

using namespace boost::python;

void export_traceTuple()
{
  class_<TraceTuple, bases<Element>, boost::shared_ptr<TraceTuple>, boost::noncopyable>
        ("TraceTuple", init<std::string, std::string>())
    .def("class_name", &TraceTuple::class_name)
    .def("processing", &TraceTuple::processing)
    .def("flow_code",  &TraceTuple::flow_code)
  ;
}
