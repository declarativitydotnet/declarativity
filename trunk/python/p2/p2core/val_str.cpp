#include <val_str.h>
#include <boost/python.hpp>
using namespace boost::python;

void export_val_str()
{
  class_<Val_Str, bases<Value>, boost::shared_ptr<Val_Str> >
        ("Val_Str", no_init)
    .def("toConfString", &Val_Str::toConfString)

    .def("mk",  &Val_Str::mk)
    .staticmethod("mk")
  ; 
}
