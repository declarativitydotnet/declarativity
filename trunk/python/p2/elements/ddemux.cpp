void (DDemux::*ro_int)(int)       = &DDemux::remove_output;
void (DDemux::*ro_vptr)(ValuePtr) = &DDemux::remove_output;

class_<DDemux, bases<Element>, boost::shared_ptr<DDemux>, boost::noncopyable>
      ("DDemux", init<std::string, std::vector<ValuePtr>, unsigned>())
  .def("class_name", &CSVParser::class_name)
  .def("processing", &CSVParser::processing)
  .def("flow_code",  &CSVParser::flow_code)
  .def("add_output", &DDemux::add_output)

  .def("remove_output", ro_int)
  .def("remove_output", ro_vptr)
;
