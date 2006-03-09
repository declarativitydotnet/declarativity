class_<ElementSpec, boost::shared_ptr<ElementSpec> >
      ("ElementSpec", init<ElementPtr>())
   .def("element", &ElementSpec::element)
;
