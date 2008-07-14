#include <tupleSourceInterface.h>
#include <boost/python.hpp>

using namespace boost::python;

void export_tupleSourceInterface()
{
  class_<TupleSourceInterface, bases<Element>, 
         boost::shared_ptr<TupleSourceInterface>, boost::noncopyable>
        ("TupleSourceInterface", init<std::string>())
    .def("class_name", &TupleSourceInterface::class_name)
    .def("processing", &TupleSourceInterface::processing)
    .def("flow_code",  &TupleSourceInterface::flow_code)
    .def("tuple",      &TupleSourceInterface::tuple)
  ;
}
