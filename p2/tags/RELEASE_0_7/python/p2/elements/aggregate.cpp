#include <boost/python.hpp>
#include <aggregate.h>

using namespace boost::python;

void export_aggregate()
{
  class_<Aggregate, bases<Element>, boost::shared_ptr<Aggregate>, boost::noncopyable>
        ("Aggregate", init<std::string, Table2::Aggregate>())
  
    .def("class_name", &Aggregate::class_name)
    .def("processing", &Aggregate::processing)
    .def("flow_code",  &Aggregate::flow_code)
  ;
}
