class_<Slot, bases<Element>, boost::shared_ptr<Slot>, boost::noncopyable>
      ("Slot", init<std::string>())
  .def("class_name", &Slot::class_name)
  .def("processing", &Slot::processing)
  .def("flow_code",  &Slot::flow_code)
;
