#include <cct.h>
#include <boost/python.hpp>

using namespace boost::python;

void export_cct()
{
  class_<CCT, bases<Element>, boost::shared_ptr<CCT>, boost::noncopyable>
      ("CCT", init<string, optional<double, double, uint, uint, uint> >())

    .def("class_name", &CCT::class_name)
    .def("processing", &CCT::processing)
    .def("flow_code",  &CCT::flow_code)
  ;
}
