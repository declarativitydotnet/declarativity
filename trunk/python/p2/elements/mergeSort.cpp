#include <mergeSort.h>
#include <boost/python.hpp>
using namespace boost::python;

void export_unmarshal()
{
  class_<MergeSort, bases<MapBase>, boost::shared_ptr<MergeSort>, boost::noncopyable>
        ("MergeSort", init<std::string>())
    .def("class_name", &Unmarshal::class_name)
    .def("processing", &Unmarshal::processing)
    .def("flow_code",  &Unmarshal::flow_code)
  ;
}
