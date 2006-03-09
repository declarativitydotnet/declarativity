class_<TimedPullPush, bases<Element>, boost::shared_ptr<TimedPullPush>, boost::noncopyable>
      ("TimedPullPush", init<std::string, double, optional<int> >())
  .def("class_name", &TimedPullPush::class_name)
  .def("processing", &TimedPullPush::processing)
  .def("flow_code",  &TimedPullPush::flow_code)

  .def("initialize", &TimedPullPush::initialize)
  .def("runTimer", &TimedPullPush:: runTimer)
;
