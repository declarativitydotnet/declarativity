class_<TrafficManager, bases<Element>, boost::shared_ptr<TrafficManager>, boost::noncopyable>
      ("TrafficManager", init<string, string, unsigned int, unsigned int, double>())
  .def("class_name", &TrafficManager::class_name)
  .def("processing", &TrafficManager::processing)
  .def("flow_code",  &TrafficManager::flow_code)
;
