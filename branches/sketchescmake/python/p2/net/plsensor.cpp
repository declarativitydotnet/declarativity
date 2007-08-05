#include <plsensor.h>
#include <boost/python.hpp>

using namespace boost::python;

void export_plsensor()
{
  class_<PlSensor, bases<Element>, boost::shared_ptr<PlSensor>, boost::noncopyable>
        ("PlSensor", init<string, uint16_t, string, uint32_t>())
  ;
}
