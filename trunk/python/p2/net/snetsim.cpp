class_<SimpleNetSim, bases<Element>, boost::shared_ptr<SimpleNetSim>, boost::noncopyable>
      ("SimpleNetSim", init<string, optional<uint32_t, uint32_t, double> >())
  .def("class_name", &SimpleNetSim::class_name)
  .def("processing", &SimpleNetSim::processing)
  .def("flow_code",  &SimpleNetSim::flow_code)
;
