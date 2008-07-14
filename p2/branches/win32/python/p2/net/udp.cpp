#include <udp.h>
#include <boost/python.hpp>

using namespace boost::python;

void export_udp()
{
 scope outer =
  class_<Udp, boost::shared_ptr<Udp>, boost::noncopyable>
        ("Udp", init<string, optional<int, int> >())
    .def("get_tx", &Udp::get_tx)
    .def("get_rx", &Udp::get_rx)
  ;

  class_<Udp::Tx, bases<Element>, boost::shared_ptr<Udp::Tx>, boost::noncopyable>
        ("UdpTx", no_init)
    .def("class_name", &Udp::Tx::class_name)
    .def("processing", &Udp::Tx::processing)
    .def("flow_code",  &Udp::Tx::flow_code)
  ;

  class_<Udp::Rx, bases<Element>, boost::shared_ptr<Udp::Rx>, boost::noncopyable>
        ("UdpRx", no_init)
    .def("class_name", &Udp::Rx::class_name)
    .def("processing", &Udp::Rx::processing)
    .def("flow_code",  &Udp::Rx::flow_code)
  ;
}
