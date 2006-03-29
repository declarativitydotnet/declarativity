#include <csvparser.h>
#include <boost/python.hpp>

using namespace boost::python;

void export_csvparser()
{
  class_<CSVParser, bases<Element>, boost::shared_ptr<CSVParser>, boost::noncopyable>
        ("CSVParser", init<std::string>())
    .def("class_name", &CSVParser::class_name)
    .def("processing", &CSVParser::processing)
    .def("flow_code",  &CSVParser::flow_code)
  ;
}
