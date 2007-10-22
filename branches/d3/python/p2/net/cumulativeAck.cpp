#include <cumulativeAck.h>
#include <boost/python.hpp>

using namespace boost::python;

void export_cumulativeAck()
{
  class_<CumulativeAck, bases<Element>, boost::shared_ptr<CumulativeAck>, boost::noncopyable>
      ("CumulativeAck", init<std::string>())
    .def("class_name", &CumulativeAck::class_name)
    .def("processing", &CumulativeAck::processing)
    .def("flow_code",  &CumulativeAck::flow_code)
  ;
}
