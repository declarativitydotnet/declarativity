#include <roundTripTimer.h>
#include <boost/python.hpp>

using namespace boost::python;

void export_roundTripTimer()
{ 
  class_<RoundTripTimer, bases<Element>, boost::shared_ptr<RoundTripTimer>, boost::noncopyable>
        ("RoundTripTimer", init<string, optional<uint, uint, uint> >())
    .def("class_name", &RoundTripTimer::class_name)
    .def("processing", &RoundTripTimer::processing)
    .def("flow_code",  &RoundTripTimer::flow_code)
  ;
}
