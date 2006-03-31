#include <elementSpec.h>
#include <boost/python.hpp>

using namespace boost::python;

void export_elementSpec()
{
  class_<ElementSpec, boost::shared_ptr<ElementSpec> >
        ("ElementSpec", init<ElementPtr>())
     .def("element",  &ElementSpec::element)
     .def("toString", &ElementSpec::toString)
  ;
}
