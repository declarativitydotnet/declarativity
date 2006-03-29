#include <ddemux.h>
#include <boost/python.hpp>

using namespace boost::python;

void (DDemux::*ro_int)(int)       = &DDemux::remove_output;
void (DDemux::*ro_vptr)(ValuePtr) = &DDemux::remove_output;

void export_ddemux()
{
  class_<DDemux, bases<Element>, boost::shared_ptr<DDemux>, boost::noncopyable>
        ("DDemux", init<std::string, std::vector<ValuePtr>, unsigned>())
    .def("class_name", &DDemux::class_name)
    .def("processing", &DDemux::processing)
    .def("flow_code",  &DDemux::flow_code)
    .def("add_output", &DDemux::add_output)
  
    .def("remove_output", ro_int)
    .def("remove_output", ro_vptr)
  ;
}
