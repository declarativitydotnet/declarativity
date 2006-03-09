class_<Sequence, bases<Element>, boost::shared_ptr<Sequence>, boost::noncopyable>
      ("Sequence", init<string, optional<uint64_t> >())
  .def("class_name", &Sequence::class_name)
  .def("processing", &Sequence::processing)
  .def("flow_code",  &Sequence::flow_code)
;
