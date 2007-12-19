#include <tuple.h>
#include <boost/python.hpp>

using namespace boost::python;

ValuePtr (Tuple::*tag1)(string)           = &Tuple::tag;
void     (Tuple::*tag2)(string, ValuePtr) = &Tuple::tag;

TuplePtr (*mk1)()       = &Tuple::mk;
TuplePtr (*mk2)(string) = &Tuple::mk;

void export_tuple()
{
  class_<Tuple, boost::shared_ptr<Tuple> >
        ("Tuple", init<>())
   .def("mk", mk1)
   .def("mk", mk2)
   .staticmethod("mk")

   .def("append",   &Tuple::append)
   .def("concat",   &Tuple::concat)
   .def("tag",      tag1)
   .def("tag",      tag2)
   .def("freeze",   &Tuple::freeze)
   .def("size",     &Tuple::size)
   .def("toString", &Tuple::toString)
   .def("at",       &Tuple::at)
  ;
}
