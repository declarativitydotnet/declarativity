#include <tuple.h>
#include <boost/python.hpp>

using namespace boost::python;

ValuePtr (Tuple::*tag1)(string)           = &Tuple::tag;
void     (Tuple::*tag2)(string, ValuePtr) = &Tuple::tag;

void export_tuple()
{
  class_<Tuple, boost::shared_ptr<Tuple> >
        ("Tuple", init<>())
   .def("mk",       &Tuple::mk)
   .staticmethod("mk")

   .def("at",       &Tuple::at)
   .def("append",   &Tuple::append)
   .def("concat",   &Tuple::concat)
   .def("tag",      tag1)
   .def("tag",      tag2)
   .def("freeze",   &Tuple::freeze)
   .def("size",     &Tuple::size)
   .def("toString", &Tuple::toString)
  ;
}
