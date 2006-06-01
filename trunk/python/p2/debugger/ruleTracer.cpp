#include <ruleTracer.h>
#include <boost/python.hpp>

using namespace boost::python;

void export_ruleTracer()
{
  class_<RuleTracer, bases<Element>, boost::shared_ptr<RuleTracer>, boost::noncopyable>
        ("RuleTracer", init<std::string, std::string, std::string,
         int, int, int, Table2Ptr, Table2Ptr>())
    .def("class_name", &RuleTracer::class_name)
    .def("processing", &RuleTracer::processing)
    .def("flow_code",  &RuleTracer::flow_code)
  ;
}
