#include <val_matrix.h>
#include <boost/python.hpp>
#include <boost/numeric/ublas/matrix.hpp>

using namespace boost::python;
using namespace boost::numeric::ublas;

void export_val_matrix()
{
  class_<Val_Matrix, bases< Value >, boost::shared_ptr<Val_Matrix> >
        ("Val_Matrix", init<size_t,size_t>())
    .def("toConfString", &Val_Matrix::toConfString)
	.def("insert", &Val_Matrix::insert)
	.def("erase", &Val_Matrix::erase)
    .def("mk2",  &Val_Matrix::mk2)
    .staticmethod("mk2")
  ; 
}
