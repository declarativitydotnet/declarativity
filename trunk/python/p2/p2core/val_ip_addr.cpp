#include <val_ip_addr.h>
#include <boost/python.hpp>
using namespace boost::python;

void export_val_ip_addr()
{
  class_<Val_IP_ADDR, bases<Value>, boost::shared_ptr<Val_IP_ADDR> >
        ("Val_IP_ADDR", no_init)
    .def("mk",  &Val_IP_ADDR::mk)
    .staticmethod("mk")
  ; 
}
