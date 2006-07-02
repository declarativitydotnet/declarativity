#include <val_vector.h>
#include <boost/python.hpp>
#include <boost/numeric/ublas/vector.hpp>

using namespace boost::python;
using namespace boost::numeric::ublas;

void export_val_vector()
{
  class_<Val_Vector, bases< vector< ValuePtr > >, boost::shared_ptr<Val_Vector> >
        ("Val_Vector", no_init)
    .def("toConfString", &Val_Vector::toConfString)
    .def("mk",  &Val_Vector::mk)
    .staticmethod("mk")
  ; 
}
