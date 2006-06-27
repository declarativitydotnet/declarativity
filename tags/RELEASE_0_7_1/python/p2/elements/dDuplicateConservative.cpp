#include <dDuplicateConservative.h>
#include <boost/python.hpp>

using namespace boost::python;

void export_dDuplicateConservative()
{
  class_<DDuplicateConservative, bases<Element>, boost::shared_ptr<DDuplicateConservative>, boost::noncopyable>
        ("DDuplicateConservative", init<std::string, int>())
    .def("class_name", &DDuplicateConservative::class_name)
    .def("processing", &DDuplicateConservative::processing)
    .def("flow_code",  &DDuplicateConservative::flow_code)
    .def("add_output", &DDuplicateConservative::add_output)
  ;
}
