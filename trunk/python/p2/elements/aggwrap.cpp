class_<Aggwrap, bases<Element>, boost::shared_ptr<Aggwrap>, boost::noncopyable>
      ("Aggwrap", init<std::string, std::string, int, std::string>())
  .def("class_name", &Aggwrap::class_name)
  .def("processing", &Aggwrap::processing)
  .def("flow_code",  &Aggwrap::flow_code)
  .def("registerGroupbyField", &Aggwrap::registerGroupbyField)
;
