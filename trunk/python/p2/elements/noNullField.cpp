class_<NoNullField, bases<Element>, boost::shared_ptr<NoNullField>, boost::noncopyable>
      ("NoNullField", init<std::string, unsigned>())
  .def("class_name", &NoNullField::class_name)
  .def("processing", &NoNullField::processing)
  .def("flow_code",  &NoNullField::flow_code)
;
