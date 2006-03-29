#include <filter.h>
#include <boost/python.hpp>
using namespace boost::python;

void export_filter()
{
  class_<Filter, bases<Element>, boost::shared_ptr<Filter>, boost::noncopyable>
        ("Filter", init<std::string, unsigned>())
    .def("class_name", &Filter::class_name)
    .def("processing", &Filter::processing)
    .def("flow_code",  &Filter::flow_code)
  ;
}
