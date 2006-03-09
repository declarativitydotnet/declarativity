class_<TimedPushSource, bases<Element>, boost::shared_ptr<TimedPushSource>, boost::noncopyable>
      ("TimedPushSource", init<string, double>())
  .def("class_name", &TimedPushSource::class_name)
  .def("flow_code",  &TimedPushSource::flow_code)
  .def("processing", &TimedPushSource::processing)

  .def("initialize", &TimedPushSource::initialize)
  .def("runTimer",   &TimedPushSource::runTimer)
;
