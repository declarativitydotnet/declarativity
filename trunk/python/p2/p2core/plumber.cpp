{
  scope outer = 
    class_<Plumber>("Plumber", init<Plumber::ConfigurationPtr, optional<LoggerI::Level> >())
      /** Initialize the engine from the configuration */
      .def("initialize", &Plumber::initialize)
  
      /** Start the plumber */
      .def("activate", &Plumber::activate)
    ;
  
    class_<Plumber::Configuration>("Configuration", init<>())
      .def(init<boost::shared_ptr<std::vector<ElementSpecPtr> >,
                boost::shared_ptr<std::vector<Plumber::HookupPtr> > >())
  
      .def("addElement", &Plumber::Configuration::addElement)
      .def("hookUp", &Plumber::Configuration::hookUp)
    ;
}
