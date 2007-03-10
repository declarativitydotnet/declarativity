#include <queue.h>
#include <boost/python.hpp>
using namespace boost::python;

void export_queue()
{
  class_<Queue, bases<Element>, boost::shared_ptr<Queue>, boost::noncopyable>
        ("Queue", init<optional<std::string, unsigned int> >())
  
    .def("class_name", &Queue::class_name)
    .def("processing", &Queue::processing)
    .def("flow_code",  &Queue::flow_code)
  ;
}
