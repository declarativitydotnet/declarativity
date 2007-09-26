#include <list.h>
#include <boost/python.hpp>

using namespace boost::python;

void export_list()
{
  class_<List, boost::shared_ptr<List> >
        ("List", init<>())
   .def("mk",       &List::mk)
   .staticmethod("mk")

   .def("member",   &List::member)
   .def("intersect",   &List::intersect)
   .def("multiset_intersect", &List::multiset_intersect)
   .def("size", &List::size)
   .def("append", &List::append)
   .def("concat", &List::concat)
   .def("toString", &List::toString)
 ;
}
