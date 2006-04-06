#include <rcct.h>
#include <boost/python.hpp>

using namespace boost::python;

void export_rcct()
{
  class_<RateCCT, bases<Element>, boost::shared_ptr<RateCCT>, boost::noncopyable>
        ("RateCCT", init<std::string, optional<bool> >())
    .def("class_name", &RateCCT::class_name)
    .def("processing", &RateCCT::processing)
    .def("flow_code",  &RateCCT::flow_code)
  ;
}
