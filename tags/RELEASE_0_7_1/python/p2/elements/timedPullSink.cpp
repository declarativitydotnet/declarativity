#include <timedPullSink.h>
#include <boost/python.hpp>
using namespace boost::python;

void export_timedPullSink()
{
  class_<TimedPullSink, bases<Element>, boost::shared_ptr<TimedPullSink>, boost::noncopyable>
        ("TimedPullSink", init<std::string, double>())
    .def("class_name", &TimedPullSink::class_name)
    .def("processing", &TimedPullSink::processing)
    .def("flow_code",  &TimedPullSink::flow_code)
  
    .def("initialize", &TimedPullSink::initialize)
    .def("runTimer", &TimedPullSink:: runTimer)
  ;
}
