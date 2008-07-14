#include <update.h>
#include <boost/python.hpp>

using namespace boost::python;

void export_update()
{
  class_<Update, bases<Element>, boost::shared_ptr<Update>, boost::noncopyable>
        ("Update", init<string, Table2Ptr>())
    .def("class_name", &Update::class_name)
    .def("processing", &Update::processing)
    .def("flow_code",  &Update::flow_code)
  ;
}
