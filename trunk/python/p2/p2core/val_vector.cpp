#include <val_vector.h>
#include <boost/python.hpp>
#include <boost/numeric/ublas/vector.hpp>

using namespace boost::python;
using namespace boost::numeric::ublas;

void export_val_vector()
{
  class_<Val_Vector, bases< Value >, boost::shared_ptr<Val_Vector> >
	    ("Val_Vector", init<size_t>())
    .def("toConfString", &Val_Vector::toConfString)
	.def("insert", &Val_Vector::insert)
	.def("erase", &Val_Vector::erase)
    .def("mk2",  &Val_Vector::mk2)
    .staticmethod("mk2")
  ; 
}
