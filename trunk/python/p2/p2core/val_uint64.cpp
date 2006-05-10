#include <val_uint64.h>
#include <boost/python.hpp>
using namespace boost::python;

void export_val_uint64()
{
  class_<Val_UInt64, bases<Value>, boost::shared_ptr<Val_UInt64> >
        ("Val_UInt64", no_init)
    .def("toConfString", &Val_UInt64::toConfString)
    .def("mk",  &Val_UInt64::mk)
    .staticmethod("mk")

    .def("cast",  &Val_UInt64::cast)
    .staticmethod("cast")
  ; 
}
