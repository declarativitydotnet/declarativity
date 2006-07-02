#include <val_matrix.h>
#include <boost/python.hpp>
#include <boost/numeric/ublas/matrix.hpp>

using namespace boost::python;
using namespace boost::numeric::ublas;

void export_val_matrix()
{
  class_<Val_Matrix, bases< matrix< ValuePtr > >, boost::shared_ptr<Val_Matrix> >
        ("Val_Matrix", no_init)
    .def("toConfString", &Val_Matrix::toConfString)
    .def("mk",  &Val_Matrix::mk)
    .staticmethod("mk")
  ; 
}
