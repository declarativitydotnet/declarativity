#include <val_tuple.h>
#include <boost/python.hpp>
using namespace boost::python;

void export_val_tuple()
{
  class_<Val_Tuple, bases<Value>, boost::shared_ptr<Val_Tuple> >
        ("Val_Tuple", no_init)
    .def("toConfString", &Val_Tuple::toConfString)
    .def("mk",  &Val_Tuple::mk)
    .staticmethod("mk")
  ; 
}
