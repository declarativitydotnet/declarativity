#include <boost/python.hpp>
#include <lookup.h>

using namespace boost::python;

void export_lookup()
{
  class_<UniqueLookup, bases<Element>, 
                boost::shared_ptr<UniqueLookup>, 
                boost::noncopyable>
        ("UniqueLookup", init<string, TablePtr, unsigned, unsigned, optional<b_cbv> >())
  
    .def("class_name", &UniqueLookup::class_name)
    .def("processing", &UniqueLookup::processing)
    .def("flow_code",  &UniqueLookup::flow_code)
  ;

  class_<MultLookup, bases<Element>, boost::shared_ptr<MultLookup>, boost::noncopyable>
        ("MultLookup", init<string, TablePtr, unsigned, unsigned, optional<b_cbv> >())
  
    .def("class_name", &MultLookup::class_name)
    .def("processing", &MultLookup::processing)
    .def("flow_code",  &MultLookup::flow_code)
  ;
}
