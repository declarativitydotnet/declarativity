#include <tupleListener.h>
#include <boost/python.hpp>

using namespace boost::python;

void export_tupleListener()
{
  class_<TupleListener, bases<Element>, boost::shared_ptr<TupleListener>, boost::noncopyable>
        ("TupleListener", init<std::string, cb_tp>())
    .def("class_name", &TupleListener::class_name)
    .def("processing", &TupleListener::processing)
    .def("flow_code",  &TupleListener::flow_code)
  ;
}
