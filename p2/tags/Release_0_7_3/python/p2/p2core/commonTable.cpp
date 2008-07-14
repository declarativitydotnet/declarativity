#include <commonTable.h>
#include <boost/python.hpp>

using namespace boost::python;

void export_commonTable()
{
  scope outer =
    class_<CommonTable, CommonTablePtr, boost::noncopyable>
    ("CommonTable", no_init)
    .def("insert", &CommonTable::insert)
    .def("size",   &CommonTable::size)
    
    .def("secondaryIndex", &CommonTable::secondaryIndex)

    .def("aggregate", &CommonTable::aggregate, return_value_policy<reference_existing_object>())
    ;
  
   class_<CommonTable::AggregateObj, CommonTable::Aggregate, boost::noncopyable>
     ("Aggregate", no_init)
   ;
}
