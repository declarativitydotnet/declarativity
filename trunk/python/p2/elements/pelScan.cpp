#include <pelScan.h>
#include <boost/python.hpp>
using namespace boost::python;

void export_pelScan()
{
  class_<PelScan, bases<Element>, boost::shared_ptr<PelScan>, boost::noncopyable>
        ("PelScan", init<string, TablePtr, unsigned, string, string, string>())
    .def("class_name", &PelScan::class_name)
    .def("processing", &PelScan::processing)
    .def("flow_code",  &PelScan::flow_code)
  ;
}
