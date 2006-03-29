#include <scan.h>
#include <boost/python.hpp>
using namespace boost::python;

void export_scan()
{
  class_<Scan, bases<Element>, boost::shared_ptr<Scan>, boost::noncopyable>
        ("Scan", init<std::string, Table::UniqueScanIterator, bool>())
    .def("class_name", &Scan::class_name)
    .def("processing", &Scan::processing)
    .def("flow_code",  &Scan::flow_code)
  ;
}
