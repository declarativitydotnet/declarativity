#include <plumber.h>
#include <boost/python.hpp>

using namespace boost::python;

void export_plumber()
{
  scope outer = 
    class_<Plumber, PlumberPtr>
          ("Plumber", init<optional<LoggerI::Level> >())
      /** Initialize the engine from the configuration */
      .def("new_dataflow_edit", &Plumber::new_dataflow_edit)
      .def("install", &Plumber::install)
      .def("toDot",   &Plumber::toDot)
    ;
  
    class_<Plumber::Dataflow, Plumber::DataflowPtr>
          ("Dataflow", init<optional<string> >())
      .def("name",       &Plumber::Dataflow::name) 
      .def("addElement", &Plumber::Dataflow::addElement)
      .def("hookUp",     &Plumber::Dataflow::hookUp)
      .def("table",      &Plumber::Dataflow::table)
    ;

    class_<Plumber::DataflowEdit, bases<Plumber::Dataflow>, Plumber::DataflowEditPtr>
          ("DataflowEdit", no_init)
      .def("name",   &Plumber::DataflowEdit::name) 
      .def("find",   &Plumber::DataflowEdit::find)
      .def("hookUp", &Plumber::Dataflow::hookUp)
    ;
}
