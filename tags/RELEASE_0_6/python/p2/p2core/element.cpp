#include <element.h>
#include <boost/python.hpp>
using namespace boost::python;

class ElementWrap : public Element, public wrapper<Element>
{
public:
  ElementWrap(std::string n) : Element(n) {};
  ElementWrap(std::string n, int i, int o) : Element(n, i, o) {};

  virtual const char* class_name() const {
    return this->get_override("class_name")();
  };
};

int (Element::*_cis)(int, Element*, int) = &Element::connect_input; 
int (Element::*_cid)(Element*, int)      = &Element::connect_input; 
int (Element::*_cos)(int, Element*, int) = &Element::connect_output; 
int (Element::*_cod)(Element*, int)      = &Element::connect_output; 

void export_element()
{
  class_<ElementWrap, boost::shared_ptr<ElementWrap>, boost::noncopyable>
        ("Element", init<std::string>())
    .def(init<std::string, int, int>())
  
    .def("class_name", pure_virtual(&Element::class_name))
  
    .def("push",           &Element::push)
    .def("pull",           &Element::pull)
    .def("simple_action",  &Element::simple_action)
  
    .def("ninputs",        &Element::ninputs)
    .def("noutputs",       &Element::noutputs)
    .def("ports_frozen",   &Element::ports_frozen)
  
    // CONFIGURATION
    .def("connect_input",  _cis)
    .def("connect_input",  _cid)
    .def("connect_output", _cos)
    .def("connect_output", _cod)
    .def("initialize",     &Element::initialize)
    
    // PROCESSING, FLOW, AND FLAGS
    .def("processing",     &Element::processing)
    .def("flow_code",      &Element::flow_code)
    .def("flags",          &Element::flags)
  ; 
}
