class_<RDelivery, bases<Element>, boost::shared_ptr<RDelivery>, boost::noncopyable>
      ("RDelivery", init<string, optional<bool, uint32_t> >())
  .def("class_name", &RDelivery::class_name)
  .def("processing", &RDelivery::processing)
  .def("flow_code",  &RDelivery::flow_code)
;
