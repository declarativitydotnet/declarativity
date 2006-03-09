class_<TupleSource, bases<FunctorSource>, boost::shared_ptr<TupleSource>, boost::noncopyable>
      ("TupleSource", init<std::string, TuplePtr>())
  .def("class_name", &TupleSource::class_name)
  .def("processing", &TupleSource::processing)
  .def("flow_code",  &TupleSource::flow_code)
;
