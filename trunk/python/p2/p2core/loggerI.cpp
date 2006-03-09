{
  scope outer = 
    class_<LoggerIWrap, boost::noncopyable>("LoggerI", no_init)
       .def("log", &LoggerIWrap::log)
    ;

    enum_<LoggerIWrap::Level>("Level")
      .value("ALL",   LoggerI::ALL)
      .value("WORDY", LoggerI::WORDY)
      .value("INFO",  LoggerI::INFO)
      .value("WARN",  LoggerI::WARN)
      .value("ERROR", LoggerI::ERROR)
      .value("NONE",  LoggerI::NONE)
    ;
}
