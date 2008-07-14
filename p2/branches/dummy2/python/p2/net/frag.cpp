#include <frag.h>
#include <boost/python.hpp>

using namespace boost::python;

void export_frag()
{
  class_<Frag, bases<Element>, boost::shared_ptr<Frag>, boost::noncopyable>
        ("Frag", init<optional<string, unsigned, unsigned> >())
    .def("class_name", &Frag::class_name)
    .def("processing", &Frag::processing)
    .def("flow_code",  &Frag::flow_code)
  ;
}
