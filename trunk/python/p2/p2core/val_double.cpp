#include <val_double.h>
#include <boost/python.hpp>
using namespace boost::python;

void export_val_double()
{
  class_<Val_Double, bases<Value>, boost::shared_ptr<Val_Double> >
        ("Val_Double", no_init)
    .def("toConfString", &Val_Double::toConfString)

    .def("mk",  &Val_Double::mk)
    .staticmethod("mk")
  ; 
}
