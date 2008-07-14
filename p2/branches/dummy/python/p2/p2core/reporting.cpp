#include <reporting.h>
#include <boost/python.hpp>

using namespace boost::python;

void export_reporting()
{
  scope in_Reporting = class_<Reporting>("Reporting", no_init)
    .def("setLevel", &Reporting::setLevel)
    .def("level", &Reporting::level)
    ;
  
  enum_<Reporting::Level>("Level")
    .value("ALL", Reporting::ALL)
    .value("WORDY", Reporting::WORDY)
    .value("INFO", Reporting::INFO)
    .value("WARN", Reporting::WARN)
    .value("ERROR", Reporting::ERROR)
    .value("OUTPUT", Reporting::OUTPUT)
    .value("NONE", Reporting::NONE)
    ;
}
