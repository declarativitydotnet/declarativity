class_<Aggregate, bases<Element>, boost::shared_ptr<Aggregate>, boost::noncopyable>
      ("Aggregate", init<std::string, Table::MultAggregate>())
  
  .def("class_name", &Aggregate::class_name)
  .def("processing", &Aggregate::processing)
  .def("flow_code",  &Aggregate::flow_code)
;
