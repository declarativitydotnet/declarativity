#include <bw.h>
#include <boost/python.hpp>

using namespace boost::python;

void export_bw()
{
  class_<Bandwidth, bases<Element>, boost::shared_ptr<Bandwidth>, boost::noncopyable>
      ("Bandwidth", init<optional<std::string> >())
    .def("class_name", &Bandwidth::class_name)
    .def("processing", &Bandwidth::processing)
    .def("flow_code",  &Bandwidth::flow_code)
  ;
}
