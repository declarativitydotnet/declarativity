    class_<Udp2, bases<Element>, boost::shared_ptr<Udp2>, boost::noncopyable>
          ("Udp2", init<string, optional<int, int> >())
      .def("class_name", &Udp2::class_name)
      .def("processing", &Udp2::processing)
      .def("flow_code",  &Udp2::flow_code)
    ;
