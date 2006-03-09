class_<Queue, bases<Element>, boost::shared_ptr<Queue>, boost::noncopyable>
      ("Queue", init<std::string, unsigned int>())

  .def("class_name", &Queue::class_name)
  .def("processing", &Queue::processing)
  .def("flow_code",  &Queue::flow_code)
;
