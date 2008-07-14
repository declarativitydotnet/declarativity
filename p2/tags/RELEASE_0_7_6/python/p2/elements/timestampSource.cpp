#include <timestampSource.h>
#include <boost/python.hpp>
using namespace boost::python;

void export_timestampSource()
{
  class_<TimestampSource, bases<Element>, boost::shared_ptr<TimestampSource>, boost::noncopyable>
        ("TimestampSource", init<std::string>())
    .def("class_name", &TimestampSource::class_name)
    .def("processing", &TimestampSource::processing)
    .def("flow_code",  &TimestampSource::flow_code)
  ;
}
