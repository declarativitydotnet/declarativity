#include <ccr.h>
#include <boost/python.hpp>

using namespace boost::python;

void export_ccr()
{
  class_<CCR, bases<Element>, boost::shared_ptr<CCR>, boost::noncopyable>
      ("CCR", init<std::string, optional<double, unsigned int, bool> >())

    .def("class_name", &CCR::class_name)
    .def("processing", &CCR::processing)
    .def("flow_code",  &CCR::flow_code)
  ;
}
