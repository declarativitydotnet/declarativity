class_<StrToSockaddr, bases<Element>, boost::shared_ptr<StrToSockaddr>, boost::noncopyable>
      ("StrToSockaddr", init<std::string, unsigned>())
  .def("class_name", &StrToSockaddr::class_name)
  .def("processing", &StrToSockaddr::processing)
  .def("flow_code",  &StrToSockaddr::flow_code)
;
