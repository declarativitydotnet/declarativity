#include <rccr.h>
#include <boost/python.hpp>

using namespace boost::python;

void export_rccr()
{
  class_<RateCCR, bases<Element>, boost::shared_ptr<RateCCR>, boost::noncopyable>
      ("RateCCR", init<std::string>())

    .def("class_name", &RateCCR::class_name)
    .def("processing", &RateCCR::processing)
    .def("flow_code",  &RateCCR::flow_code)
  ;
}
