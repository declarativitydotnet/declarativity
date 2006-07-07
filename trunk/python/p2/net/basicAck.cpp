#include <basicAck.h>
#include <boost/python.hpp>

using namespace boost::python;

void export_basicAck()
{
  class_<BasicAck, bases<Element>, boost::shared_ptr<BasicAck>, boost::noncopyable>
      ("BasicAck", init<std::string, optional<uint, uint, uint> >())
    .def("class_name", &BasicAck::class_name)
    .def("processing", &BasicAck::processing)
    .def("flow_code",  &BasicAck::flow_code)
  ;
}
