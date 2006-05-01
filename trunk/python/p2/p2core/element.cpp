#include <element.h>
#include <loop.h>
#include <boost/python.hpp>
using namespace boost::python;
using namespace boost::python::api;

class ElementWrap : public Element, public wrapper<Element>
{
public:
/*
  ElementWrap(std::string n) : Element(n) {};
  ElementWrap(std::string n, int i, int o) : Element(n, i, o) {};
  virtual const char* class_name() const {
    return this->get_override("class_name")();
  }
*/
  ElementWrap(std::string n) : Element(n), tHandle_(NULL) {};
  ElementWrap(std::string n, int i, int o) : Element(n, i, o), tHandle_(NULL) {};

  int py_push(int port, TuplePtr tp) {
    return output(port)->push(tp, boost::bind(&ElementWrap::callback, this));
  }

  TuplePtr py_pull(int port) {
    return input(port)->pull(boost::bind(&ElementWrap::callback, this));
  }

  virtual const char *processing() const {
    if (override f = this->get_override("processing")) {
      return f();
    }
    return Element::processing();
  }

  virtual const char *flow_code() const {
    if (override f = this->get_override("flow_code")) {
      return f();
    }
    return Element::flow_code();
  }

  virtual const char* class_name() const {
    return this->get_override("class_name")();
  }

  virtual int initialize() {
    if (override f = this->get_override("initialize")) {
      return f();
    }
    return Element::initialize();
  }

  virtual int push(int port, TuplePtr tp, b_cbv cb) {
    if (override f = this->get_override("push")) {
      return f(port, tp, cb);
    }
    return Element::push(port, tp, cb);
  }

  virtual TuplePtr pull(int port, b_cbv cb) {
    if (override f = this->get_override("pull")) {
      return f(port, cb);
    }
    return Element::pull(port, cb);
  }

  virtual TuplePtr simple_action(TuplePtr p) {
    if (override f = this->get_override("simple_action")) {
      return f(p);
    }
    return Element::simple_action(p);
  }

  virtual void callback() {
    if (override f = this->get_override("callback")) {
      f();
    }
  }
  virtual void delay_callback() {
    if (override f = this->get_override("delay_callback")) {
      f();
    }
  }

  void set_delay(double secondDelay) {
    if (tHandle_) cancel_delay();
    delayCB(secondDelay, boost::bind(&ElementWrap::delay_callback, this));
  }
  void cancel_delay() {
    if (tHandle_) timeCBRemove(tHandle_);
    tHandle_ = NULL;
  }

private:
  timeCBHandle* tHandle_;
};

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
    .def("connect_input",  &Element::connect_input)
    .def("connect_output", &Element::connect_output)
    .def("initialize",     &Element::initialize)
    
    // PROCESSING, FLOW, AND FLAGS
    .def("processing",     &Element::processing)
    .def("flow_code",      &Element::flow_code)
    .def("flags",          &Element::flags)

    // PYTHON SUPPORT FOR ELEMENTS
    .def("push",           &ElementWrap::push)
    .def("pull",           &ElementWrap::pull)
    .def("simple_action",  &ElementWrap::simple_action)
    .def("py_push",        &ElementWrap::py_push)
    .def("py_pull",        &ElementWrap::py_pull)
    .def("callback",       &ElementWrap::callback)
    .def("delay_callback", &ElementWrap::delay_callback)
    .def("set_delay",      &ElementWrap::set_delay)
    .def("cancel_delay",   &ElementWrap::cancel_delay)
  ; 
}
