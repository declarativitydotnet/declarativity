#include <duplicate.h>
#include <boost/python.hpp>

using namespace boost::python;

void export_duplicate()
{
  class_<Duplicate, bases<Element>, boost::shared_ptr<Duplicate>, boost::noncopyable>
        ("Duplicate", init<std::string, int>())
    .def("class_name", &Duplicate::class_name)
    .def("processing", &Duplicate::processing)
    .def("flow_code",  &Duplicate::flow_code)
  ;
}
