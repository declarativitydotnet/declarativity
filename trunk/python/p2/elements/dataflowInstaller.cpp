#include <dataflowInstaller.h>
#include <boost/python.hpp>
using namespace boost::python;

void export_dataflowInstaller()
{
  class_<DataflowInstaller, bases<Element>, boost::shared_ptr<DataflowInstaller>, boost::noncopyable>
        ("DataflowInstaller", init<std::string, PlumberPtr, optional<boost::python::api::object> >())
    .def("class_name", &DataflowInstaller::class_name)
    .def("processing", &DataflowInstaller::processing)
    .def("flow_code",  &DataflowInstaller::flow_code)
  ;
}
