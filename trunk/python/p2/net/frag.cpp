class_<Frag, bases<Element>, boost::shared_ptr<Frag>, boost::noncopyable>
      ("Frag", init<string, uint32_t>())
  .def("class_name", &Frag::class_name)
  .def("processing", &Frag::processing)
  .def("flow_code",  &Frag::flow_code)
;
