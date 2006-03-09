class_<Logger, bases<Element, LoggerI>, boost::shared_ptr<Logger>, boost::noncopyable>
      ("Logger", init<std::string>())
  .def("class_name", &Delete::class_name)
  .def("processing", &Delete::processing)
  .def("flow_code",  &Delete::flow_code)
  
  /** Override this since it's pure virtual in the interface */
  .def("log", &Logger::log)
;
