class_<RandomPushSource, bases<Element>, boost::shared_ptr<RandomPushSource>, boost::noncopyable>
      ("RandomPushSource", init<std::string, double, int, int>())

  .def("class_name", &RandomPushSource::class_name)
  .def("processing", &RandomPushSource::processing)
  .def("flow_code",  &RandomPushSource::flow_code)

  .def("initialize", &RandomPushSource::initialize)
  .def("runTimer",   &RandomPushSource::runTimer)
;
