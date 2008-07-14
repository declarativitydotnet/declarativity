#include <hexdump.h>
#include <boost/python.hpp>
using namespace boost::python;

void export_hexdump()
{
  class_<Hexdump, bases<Element>, boost::shared_ptr<Hexdump>, boost::noncopyable>
        ("Hexdump", init<std::string, unsigned>())
    .def("class_name", &Hexdump::class_name)
    .def("processing", &Hexdump::processing)
    .def("flow_code",  &Hexdump::flow_code)
  ;
}
