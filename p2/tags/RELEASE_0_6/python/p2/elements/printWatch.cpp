#include <printWatch.h>
#include <boost/python.hpp>
using namespace boost::python;

void export_printWatch()
{
  class_<PrintWatch, bases<Element>, boost::shared_ptr<PrintWatch>, boost::noncopyable>
        ("PrintWatch", init<std::string, std::set<std::string> >())
    .def("class_name", &PrintWatch::class_name)
    .def("processing", &PrintWatch::processing)
    .def("flow_code",  &PrintWatch::flow_code)
  ;
}
