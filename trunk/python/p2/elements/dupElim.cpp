class_<DupElim, bases<Element>, boost::shared_ptr<DupElim>, boost::noncopyable>
      ("DupElim", init<std::string>())
  .def("class_name", &DupElim::class_name)
  .def("processing", &DupElim::processing)
  .def("flow_code",  &DupElim::flow_code)
;
