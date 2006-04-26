#include <val_null.h>
#include <boost/python.hpp>
using namespace boost::python;

void export_val_null()
{
  class_<Val_Null, bases<Value>, boost::shared_ptr<Val_Null> >
        ("Val_Null", no_init)
    .def("toConfString", &Val_Null::toConfString)

    .def("mk",  &Val_Null::mk)
    .staticmethod("mk")
  ; 
}
