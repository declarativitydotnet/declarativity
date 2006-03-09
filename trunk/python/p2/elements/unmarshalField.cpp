class_<UnmarshalField, bases<Element>, boost::shared_ptr<UnmarshalField>, boost::noncopyable>
      ("UnmarshalField", init<std::string, unsigned int>())

  .def("class_name", &UnmarshalField::class_name)
  .def("processing", &UnmarshalField::processing)
  .def("flow_code",  &UnmarshalField::flow_code)
;
