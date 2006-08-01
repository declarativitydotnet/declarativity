#include <insert.h>
#include <boost/python.hpp>
using namespace boost::python;

void export_insert()
{
  class_<Insert, bases<Element>, boost::shared_ptr<Insert>, boost::noncopyable>
        ("Insert", init<std::string, CommonTablePtr>())
    .def("class_name", &Insert::class_name)
    .def("processing", &Insert::processing)
    .def("flow_code",  &Insert::flow_code)
  ;
}
