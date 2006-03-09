class_<Delete, bases<Element>, boost::shared_ptr<Delete>, boost::noncopyable>
      ("Delete", init<string, TablePtr, unsigned, unsigned>())
  .def("class_name", &Delete::class_name)
  .def("processing", &Delete::processing)
  .def("flow_code",  &Delete::flow_code)
;
