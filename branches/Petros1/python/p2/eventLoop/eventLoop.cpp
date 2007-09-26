#include <loop.h>
#include <boost/python.hpp>

using namespace boost::python;

void export_eventLoop()
{
  class_<timeCBHandle, timeCBHandle*>
        ("timeCBHandle", init<boost::posix_time::ptime&, const b_cbv&, Element*>())
  ;
  
  enum_<b_selop>("b_selop")
    .value("b_selread", b_selread)
    .value("b_selwrite", b_selwrite)
  ;
  
  def("eventLoop", eventLoop);
  def("eventLoopInitialize", eventLoopInitialize);
/*
  def("delayCB", delayCB, return_internal_reference<>())
  def("timeCBRemove", timeCBRemove)
  def("networkSocket", networkSocket)
  def("fileDescriptorCB", fileDescriptorCB)
  def("removeFileDescriptorCB", removeFileDescriptorCB)
*/
}
