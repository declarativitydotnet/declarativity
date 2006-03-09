class_<Joiner, bases<Element>, boost::shared_ptr<Joiner>, boost::noncopyable>
      ("Joiner", init<std::string>())
  .def("class_name", &Joiner::class_name)
  .def("processing", &Joiner::processing)
  .def("flow_code",  &Joiner::flow_code)
;
