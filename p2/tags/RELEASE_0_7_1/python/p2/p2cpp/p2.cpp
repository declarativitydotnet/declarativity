#include <p2.h>
#include <boost/python.hpp>
#include <boost/python/module.hpp>
#include <boost/python/class.hpp>
#include <boost/utility.hpp>
using namespace boost::python;
using namespace boost::python::api;

class P2Wrap : public P2, public wrapper<P2>
{
public:
  P2Wrap(std::string hostname, std::string port)
    : P2(hostname, port) {};

  CallbackHandlePtr subscribe(string tableName, object callback) {
    return P2::subscribe(tableName, boost::function<void (TuplePtr)>(callback));
  }
};

void export_p2()
{
scope outer =
  class_<P2Wrap, boost::shared_ptr<P2Wrap>, boost::noncopyable>
        ("P2", init<string, string>())
    .def("run",         &P2::run)
    .def("install",     &P2::install)
    .def("subscribe",   &P2Wrap::subscribe)
    .def("unsubscribe", &P2::unsubscribe)
    .def("tuple",       &P2::tuple)
  ;

  class_<P2::CallbackHandle, P2::CallbackHandlePtr, boost::noncopyable>
        ("CallbackHandle", no_init)
  ;
}
