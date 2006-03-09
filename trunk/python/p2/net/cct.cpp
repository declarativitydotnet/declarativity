class_<CCT, bases<Element>, boost::shared_ptr<CCT>, boost::noncopyable>
      ("CCT", init<string, double, double, optional<bool, bool> >())

    .def("class_name", &CCT::class_name)
    .def("processing", &CCT::processing)
    .def("flow_code",  &CCT::flow_code)
;
