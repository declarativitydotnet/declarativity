#include <mergeSort.h>
#include <boost/python.hpp>
using namespace boost::python;

void export_mergeSort()
{
  class_<MergeSort, bases<MapBase>, boost::shared_ptr<MergeSort>, boost::noncopyable>
        ("MergeSort", init<std::string>())
    .def("class_name", &MergeSort::class_name)
    .def("processing", &MergeSort::processing)
    .def("flow_code",  &MergeSort::flow_code)
  ;
}
