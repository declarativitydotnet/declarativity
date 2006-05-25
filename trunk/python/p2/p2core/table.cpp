#include <table.h>
#include <boost/python.hpp>

using namespace boost::python;

void (Table::*add_uni_index1)(unsigned)               
  = &Table::add_unique_index;
void (Table::*add_uni_index2)(std::vector<unsigned>)  
  = &Table::add_unique_index;
void (Table::*del_uni_index1)(unsigned)               
  = &Table::del_unique_index;
void (Table::*del_uni_index2)(std::vector<unsigned>)  
  = &Table::del_unique_index;
void (Table::*add_mult_index1)(unsigned)              
  = &Table::add_multiple_index;
void (Table::*add_mult_index2)(std::vector<unsigned>) 
  = &Table::add_multiple_index;
void (Table::*del_mult_index1)(unsigned)              
  = &Table::del_multiple_index;
void (Table::*del_mult_index2)(std::vector<unsigned>) 
  = &Table::del_multiple_index;

Table::MultAggregate (Table::*amga2)(std::vector<unsigned>, std::vector<unsigned>, 
                     unsigned, Table::AggregateFunction&) 
  = &Table::add_mult_groupBy_agg;

void export_table()
{
scope outer = 
  class_<Table, TablePtr>
        ("Table", init<string, size_t>())
     .def(init<string, size_t, string>())

     .def("insert", &Table::insert)
     .def("size",   &Table::size)

     .def("add_unique_index",    add_uni_index1)
     .def("add_unique_index",    add_uni_index2)
     .def("del_unique_index",    del_uni_index1)
     .def("del_unique_index",    del_uni_index2)
     .def("add_multiple_index",  add_mult_index1)
     .def("add_multiple_index",  add_mult_index2)
     .def("del_multiple_index",  del_mult_index1)
     .def("del_multiple_index",  del_mult_index2)

     .def("add_mult_groupBy_agg",   amga2)

     .def("agg_min",   &Table::agg_min, return_value_policy<reference_existing_object>())
     .def("agg_max",   &Table::agg_max, return_value_policy<reference_existing_object>())
     .def("agg_count", &Table::agg_count, return_value_policy<reference_existing_object>())
     .staticmethod("agg_min")
     .staticmethod("agg_max")
     .staticmethod("agg_count")
  ;

  class_<Table::AggregateFunction, Table::AggregateFunction*, boost::noncopyable>
        ("AggregateFunction", no_init)
  ;

  class_<Table::AggregateObj<Table::MultIndex>, Table::MultAggregate>
        ("MultAggregate", init<std::vector<unsigned>, Table::MultIndex*, std::vector<unsigned>, 
                        unsigned, Table::AggregateFunction&>())
    .def("addListener", &Table::AggregateObj<Table::MultIndex>::addListener)
    .def("update",      &Table::AggregateObj<Table::MultIndex>::update)
  ;
}
