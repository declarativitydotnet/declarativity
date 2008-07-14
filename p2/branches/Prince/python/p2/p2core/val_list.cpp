#include <val_list.h>
#include <boost/python.hpp>
using namespace boost::python;

void export_val_list()
{
  class_<Val_List, bases<Value>, boost::shared_ptr<Val_List> >
        ("Val_List", init<ListPtr>())
    .def("toConfString", &Val_List::toConfString)
    .def("mk",  &Val_List::mk)
    .staticmethod("mk")
  ; 
}
