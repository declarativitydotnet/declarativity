#include <odelivery.h>
#include <boost/python.hpp>

using namespace boost::python;

void export_odelivery()
{ 
  class_<ODelivery, bases<Element>, boost::shared_ptr<ODelivery>, boost::noncopyable>
        ("ODelivery", init<string, optional<uint, uint, uint> >())
    .def("class_name", &ODelivery::class_name)
    .def("processing", &ODelivery::processing)
    .def("flow_code",  &ODelivery::flow_code)
  ;
}
