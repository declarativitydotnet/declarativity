class_<PelScan, bases<Element>, boost::shared_ptr<PelScan>, boost::noncopyable>
      ("PelScan", init<string, TablePtr, unsigned, string, string, string>())
  .def("class_name", &Delete::class_name)
  .def("processing", &Delete::processing)
  .def("flow_code",  &Delete::flow_code)
;
