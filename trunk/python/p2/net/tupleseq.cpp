#include <tupleseq.h>
#include <boost/python.hpp>

using namespace boost::python;

void export_tupleseq()
{
  class_<Sequence, bases<Element>, boost::shared_ptr<Sequence>, boost::noncopyable>
        ("Sequence", init<optional<string, uint64_t> >())
    .def("class_name", &Sequence::class_name)
    .def("processing", &Sequence::processing)
    .def("flow_code",  &Sequence::flow_code)
  ;
}
