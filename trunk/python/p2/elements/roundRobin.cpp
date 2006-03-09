class_<RoundRobin, bases<Element>, boost::shared_ptr<RoundRobin>, boost::noncopyable>
      ("RoundRobin", init<std::string, int>())

  .def("class_name", &RoundRobin::class_name)
  .def("processing", &RoundRobin::processing)
  .def("flow_code",  &RoundRobin::flow_code)
;
