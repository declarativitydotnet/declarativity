#include <elementSpec.h>
#include <boost/python.hpp>

using namespace boost::python;

BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(add_inputs, add_input, 0, 1)
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(add_outputs, add_output, 0, 1)

void (ElementSpec::*ri_i)(int)      = &ElementSpec::remove_input;
void (ElementSpec::*ri_v)(ValuePtr) = &ElementSpec::remove_input;
void (ElementSpec::*ro_i)(int)      = &ElementSpec::remove_output;
void (ElementSpec::*ro_v)(ValuePtr) = &ElementSpec::remove_output;

void export_elementSpec()
{
scope outer = 
  class_<ElementSpec, boost::shared_ptr<ElementSpec> >
        ("ElementSpec", init<ElementPtr>())
     .def("element",  &ElementSpec::element)
     .def("toString", &ElementSpec::toString)

     .def("add_input", &ElementSpec::add_input, add_inputs())
     .def("add_output", &ElementSpec::add_output, add_outputs())

     .def("remove_input",  ri_i)
     .def("remove_input",  ri_v)
     .def("remove_output", ri_i)
     .def("remove_output", ri_v)
  ;

  class_<ElementSpec::Hookup, ElementSpec::HookupPtr>
        ("Hookup", init<ElementSpecPtr, int, ElementSpecPtr, int>())
  ;
}
