#include <val_id.h>
#include <boost/python.hpp>
using namespace boost::python;

void export_val_id()
{
  class_<Val_ID, bases<Value>, boost::shared_ptr<Val_ID> >
        ("Val_ID", no_init)
    .def("mk",  &Val_ID::mk)
    .staticmethod("mk")
  ; 
}
