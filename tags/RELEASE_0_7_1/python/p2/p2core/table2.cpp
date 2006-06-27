#include <table2.h>
#include <boost/python.hpp>

using namespace boost::python;

void export_table2()
{
  scope outer =
    class_<Table2, Table2Ptr>
    ("Table2", init<string, Table2::Key&>())
    .def(init<string, Table2::Key&, uint32_t>())
    .def(init<string, Table2::Key&, uint32_t, string>())
    
    .def("insert", &Table2::insert)
    .def("size",   &Table2::size)
    
    .def("secondaryIndex", &Table2::secondaryIndex)

    .def("aggregate", &Table2::aggregate, return_value_policy<reference_existing_object>())
    ;
  
   class_<Table2::AggregateObj, Table2::Aggregate, boost::noncopyable>
     ("Aggregate", no_init)
   ;
}
