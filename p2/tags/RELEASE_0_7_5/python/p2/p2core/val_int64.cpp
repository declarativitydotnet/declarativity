#include <val_int64.h>
#include <boost/python.hpp>
using namespace boost::python;

void export_val_int64()
{
  class_<Val_Int64, bases<Value>, boost::shared_ptr<Val_Int64> >
        ("Val_Int64", no_init)
    .def("toConfString", &Val_Int64::toConfString)

    .def("mk",  &Val_Int64::mk)
    .staticmethod("mk")

    .def("cast",  &Val_Int64::cast)
    .staticmethod("cast")
  ; 
}
