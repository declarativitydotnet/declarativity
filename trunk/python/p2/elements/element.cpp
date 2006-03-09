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
; 
