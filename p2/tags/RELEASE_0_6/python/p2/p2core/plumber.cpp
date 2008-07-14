#include <plumber.h>
#include <boost/python.hpp>

using namespace boost::python;

ElementSpecPtr (Plumber::Dataflow::*add1)(ElementPtr)     = &Plumber::Dataflow::addElement;
ElementSpecPtr (Plumber::Dataflow::*add2)(string, string) = &Plumber::Dataflow::addElement;

void export_plumber()
{
  scope outer = 
    class_<Plumber, PlumberPtr>
          ("Plumber", init<optional<LoggerI::Level> >())
      /** Initialize the engine from the configuration */
      .def("new_dataflow", &Plumber::new_dataflow)
      .def("install", &Plumber::install)
      .def("toDot", &Plumber::toDot)
    ;
  
    class_<Plumber::Dataflow, Plumber::DataflowPtr>
          ("Dataflow", no_init)
      .def("addElement", add1)
      .def("addElement", add2)
      .def("hookUp", &Plumber::Dataflow::hookUp)
    ;
}
