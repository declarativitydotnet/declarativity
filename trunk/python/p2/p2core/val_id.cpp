#include <val_id.h>
#include <boost/python.hpp>
using namespace boost::python;

ValuePtr (*mk_id)   (IDPtr)                 = &Val_ID::mk; 
ValuePtr (*mk_vec)  (std::vector<uint32_t>) = &Val_ID::mk;
ValuePtr (*mk_int32)(uint32_t)              = &Val_ID::mk;
ValuePtr (*mk_int64)(uint64_t)              = &Val_ID::mk;

void export_val_id()
{
  class_<Val_ID, bases<Value>, boost::shared_ptr<Val_ID> >
        ("Val_ID", no_init)
    .def("toConfString", &Val_ID::toConfString)
    .def("mk", mk_id)
    .def("mk", mk_vec)
    .def("mk", mk_int32)
    .def("mk", mk_int64)
    .staticmethod("mk")
  ; 
}
