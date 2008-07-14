#include <val_time.h>
#include <boost/python.hpp>
using namespace boost::python;

ValuePtr (*mk_ptime)(boost::posix_time::ptime) = &Val_Time::mk;
ValuePtr (*mk_timespec)(struct timespec)       = &Val_Time::mk;

void export_val_time()
{
  class_<Val_Time, bases<Value>, boost::shared_ptr<Val_Time> >
        ("Val_Time", no_init)
    .def("mk",  mk_ptime)
    .def("mk",  mk_timespec)
    .staticmethod("mk")
  ; 
}
