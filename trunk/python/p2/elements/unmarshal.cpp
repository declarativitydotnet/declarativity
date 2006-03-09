class_<Unmarshal, bases<Element>, boost::shared_ptr<Unmarshal>, boost::noncopyable>
      ("Unmarshal", init<std::string>())
  .def("class_name", &Unmarshal::class_name)
  .def("processing", &Unmarshal::processing)
  .def("flow_code",  &Unmarshal::flow_code)
;
