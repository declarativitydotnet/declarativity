#include <p2.h>
#include <boost/python.hpp>

using namespace boost::python;

void export_p2()
{
scope outer =
  class_<P2, boost::shared_ptr<P2>, boost::noncopyable>
        ("P2", init<string, string>())
    .def("run",         &P2::run)
    .def("install",     &P2::install)
    .def("subscribe",   &P2::subscribe)
    .def("unsubscribe", &P2::unsubscribe)
    .def("tuple",       &P2::tuple)
  ;

  class_<P2::CallbackHandle, P2::CallbackHandlePtr, boost::noncopyable>
        ("CallbackHandle", no_init)
  ;
}
