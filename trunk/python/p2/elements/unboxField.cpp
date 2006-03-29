#include <unboxField.h>
#include <boost/python.hpp>
using namespace boost::python;

void export_unboxField()
{
  class_<UnboxField, bases<Element>, boost::shared_ptr<UnboxField>, boost::noncopyable>
        ("UnboxField", init<std::string, unsigned int>())
    .def("class_name", &UnboxField::class_name)
    .def("processing", &UnboxField::processing)
    .def("flow_code",  &UnboxField::flow_code)
  ;
}
