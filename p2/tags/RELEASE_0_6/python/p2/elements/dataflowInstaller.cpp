#include <dataflowInstaller.h>
#include <boost/python.hpp>
using namespace boost::python;

void (DataflowInstaller::*install_d)(Plumber::DataflowPtr) = &DataflowInstaller::install;
void (DataflowInstaller::*install_f)(string)               = &DataflowInstaller::install;

void export_dataflowInstaller()
{
  class_<DataflowInstaller, bases<Element>, boost::shared_ptr<DataflowInstaller>, boost::noncopyable>
        ("DataflowInstaller", init<std::string, PlumberPtr, optional<boost::python::api::object> >())
    .def("class_name", &DataflowInstaller::class_name)
    .def("processing", &DataflowInstaller::processing)
    .def("flow_code",  &DataflowInstaller::flow_code)
    .def("install",    install_f)
    .def("install",    install_d)
  ;
}
