#include <rdelivery.h>
#include <boost/python.hpp>

using namespace boost::python;

void export_rdelivery()
{ 
  class_<RDelivery, bases<Element>, boost::shared_ptr<RDelivery>, boost::noncopyable>
        ("RDelivery", init<string, optional<uint> >())
    .def("class_name", &RDelivery::class_name)
    .def("processing", &RDelivery::processing)
    .def("flow_code",  &RDelivery::flow_code)
  ;
}
