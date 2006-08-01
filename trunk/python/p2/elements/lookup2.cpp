#include <boost/python.hpp>
#include <lookup2.h>

using namespace boost::python;

void export_lookup2()
{
  class_<Lookup2, bases<Element>, 
                boost::shared_ptr<Lookup2>, 
                boost::noncopyable>
    ("Lookup2", init<string, CommonTablePtr, CommonTable::Key, CommonTable::Key, optional<b_cbv> >())
    .def("class_name", &Lookup2::class_name)
    .def("processing", &Lookup2::processing)
    .def("flow_code",  &Lookup2::flow_code)
  ;

}
