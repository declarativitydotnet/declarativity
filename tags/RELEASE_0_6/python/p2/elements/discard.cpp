#include <discard.h>
#include <boost/python.hpp>

using namespace boost::python;

void export_discard()
{
  class_<Discard, bases<Element>, boost::shared_ptr<Discard>, boost::noncopyable>
        ("Discard", init<std::string>())
    .def("class_name", &Discard::class_name)
    .def("processing", &Discard::processing)
    .def("flow_code",  &Discard::flow_code)
  ;
}
