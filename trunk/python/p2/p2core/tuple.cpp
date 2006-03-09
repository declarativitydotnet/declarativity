ValuePtr (Tuple::*tag1)(string)           = &Tuple::tag;
void     (Tuple::*tag2)(string, ValuePtr) = &Tuple::tag;

class_<Tuple, boost::shared_ptr<Tuple> >
      ("Tuple", init<>())
   .def("mk",       &Tuple::mk)
   .def("append",   &Tuple::append)
   .def("concat",   &Tuple::concat)
   .def("tag",      tag1)
   .def("tag",      tag2)
   .def("freeze",   &Tuple::freeze)
   .def("size",     &Tuple::size)
   .def("toString", &Tuple::toString)
;
