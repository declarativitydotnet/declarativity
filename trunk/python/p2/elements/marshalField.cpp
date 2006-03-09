class_<MarshalField, bases<Element>, boost::shared_ptr<MarshalField>, boost::noncopyable>
      ("MarshalField", init<std::string, unsigned>())
  .def("class_name", &MarshalField::class_name)
  .def("processing", &MarshalField::processing)
  .def("flow_code",  &MarshalField::flow_code)
;
