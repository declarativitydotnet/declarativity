#include <val_opaque.h>
#include <boost/python.hpp>
using namespace boost::python;

void export_val_opaque()
{
  class_<Val_Opaque, bases<Value>, boost::shared_ptr<Val_Opaque> >
        ("Val_Opaque", no_init)
    .def("toConfString", &Val_Opaque::toConfString)

    .def("mk",  &Val_Opaque::mk)
    .staticmethod("mk")
  ; 
}
