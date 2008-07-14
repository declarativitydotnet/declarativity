#include <mux.h>
#include <boost/python.hpp>
using namespace boost::python;

void export_mux()
{
  class_<Mux, bases<Element>, boost::shared_ptr<Mux>, boost::noncopyable>
        ("Mux", init<std::string, int>())
    .def("class_name", &Mux::class_name)
    .def("processing", &Mux::processing)
    .def("flow_code",  &Mux::flow_code)
  ;
}
