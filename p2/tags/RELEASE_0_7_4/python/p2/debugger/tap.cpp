#include <tap.h>
#include <boost/python.hpp>

using namespace boost::python;

void export_tap()
{
  class_<Tap, bases<Element>, boost::shared_ptr<Tap>, boost::noncopyable>
        ("Tap", init<std::string, int>())
    .def("class_name", &Tap::class_name)
    .def("processing", &Tap::processing)
    .def("flow_code",  &Tap::flow_code)
  ;
}
