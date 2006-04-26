#include <val_uint32.h>
#include <boost/python.hpp>
using namespace boost::python;

void export_val_uint32()
{
  class_<Val_UInt32, bases<Value>, boost::shared_ptr<Val_UInt32> >
        ("Val_UInt32", no_init)
    .def("toConfString", &Val_UInt32::toConfString)
    .def("mk",  &Val_UInt32::mk)
    .staticmethod("mk")
  ; 
}
