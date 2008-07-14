#include <dRoundRobin.h>
#include <boost/python.hpp>

using namespace boost::python;

void export_dRoundRobin()
{
  class_<DRoundRobin, bases<Element>, boost::shared_ptr<DRoundRobin>, boost::noncopyable>
        ("DRoundRobin", init<std::string, int>())
    .def("class_name",    &DRoundRobin::class_name)
    .def("processing",    &DRoundRobin::processing)
    .def("flow_code",     &DRoundRobin::flow_code)

    .def("add_input",     &DRoundRobin::add_input)
    .def("remove_input",  &DRoundRobin::remove_input)
  ;
}
