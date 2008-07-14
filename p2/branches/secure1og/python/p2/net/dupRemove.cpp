#include <dupRemove.h>
#include <boost/python.hpp>

using namespace boost::python;

void export_dupRemove()
{ 
  class_<DupRemove, bases<Element>, boost::shared_ptr<DupRemove>, boost::noncopyable>
        ("DupRemove", init<string>())
    .def("class_name", &DupRemove::class_name)
    .def("processing", &DupRemove::processing)
    .def("flow_code",  &DupRemove::flow_code)
  ;
}
