/*
* Copyright (c) 2003 Intel Corporation
* All rights reserved.
*
*/

#include <vector>
#include <ctime>
#include <string>
#include <boost/python.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>

using namespace boost::python;

void export_aggregate();
void export_aggwrap();
void export_csvparser();
void export_dataflowInstaller();
void export_delete();
void export_update();
void export_lookup2();
void export_demux();
void export_ddemux();
void export_discard();
void export_dupElim();
void export_duplicateConservative();
void export_dDuplicateConservative();
void export_duplicate();
void export_element();
void export_filter();
void export_functorSource();
void export_hexdump();
void export_insert();
void export_logger();
void export_marshal();
void export_marshalField();
void export_mux();
void export_noNull();
void export_noNullField();
void export_pelTransform();
void export_print();
void export_printTime();
void export_printWatch();
void export_queue();
void export_roundRobin();
void export_dRoundRobin();
void export_slot();
void export_strToSockkaddr();
void export_timedPullPush();
void export_timedPullSink();
void export_timedPushSource();
void export_timestampSource();
void export_tupleSource();
void export_unboxField();
void export_unmarshal();
void export_unmarshalField();
void export_eventLoop();
void export_bw();
void export_ccr();
void export_cct();
void export_rccr();
void export_rcct();
void export_defrag();
void export_frag();
void export_plsensor();
void export_rdelivery();
void export_skr();
void export_snetsim();
void export_tman();
void export_tupleseq();
void export_udp();
void export_udp2();
void export_elementSpec();
void export_loggerI();
void export_plumber();
void export_tuple();
void export_table2();
void export_value();
void export_val_double();
void export_val_id();
void export_val_int32();
void export_val_int64();
void export_val_ip_addr();
void export_val_null();
void export_val_opaque();
void export_val_str();
void export_val_time();
void export_val_tuple();
void export_val_uint32();
void export_val_uint64();
void export_overlogCompiler();

time_t time_v(void)
{
  return time(NULL);
}

BOOST_PYTHON_MODULE(libp2python)
{
  def("srand", srand);
  def("time",  time_v);

  class_<std::vector<std::string>, boost::shared_ptr<std::vector<std::string> > >("StrVec")
    .def(vector_indexing_suite<std::vector<std::string>, true>())
  ;
  class_<std::vector<unsigned>, boost::shared_ptr<std::vector<unsigned> > >("IntVec")
    .def(vector_indexing_suite<std::vector<unsigned> >())
  ;

  class_<boost::function<void (void)> >("b_cbv", no_init)
  ;

  export_element();
  export_elementSpec();
  export_loggerI();
  export_tuple();
  export_table2();
  export_plumber();
  export_value();
  export_val_double();
  export_val_id();
  export_val_int32();
  export_val_int64();
  export_val_ip_addr();
  export_val_null();
  export_val_opaque();
  export_val_str();
  export_val_time();
  export_val_tuple();
  export_val_uint32();
  export_val_uint64();

  export_aggregate();
  export_aggwrap();
  export_csvparser();
  export_dataflowInstaller();
  export_delete();
  export_update();
  export_lookup2();
  export_demux();
  export_ddemux();
  export_discard();
  export_dupElim();
  export_duplicateConservative();
  export_dDuplicateConservative();
  export_duplicate();
  export_filter();
  export_functorSource();
  export_hexdump();
  export_insert();
  export_logger();
  export_marshal();
  export_marshalField();
  export_mux();
  export_noNull();
  export_noNullField();
  export_pelTransform();
  export_print();
  export_printTime();
  export_printWatch();
  export_queue();
  export_roundRobin();
  export_dRoundRobin();
  export_slot();
  export_strToSockkaddr();
  export_timedPullPush();
  export_timedPullSink();
  export_timedPushSource();
  export_timestampSource();
  export_tupleSource();
  export_unboxField();
  export_unmarshal();
  export_unmarshalField();
  export_eventLoop();
  export_bw();
  export_ccr();
  export_cct();
  export_defrag();
  export_frag();
  export_plsensor();
  export_rccr();
  export_rcct();
  export_rdelivery();
  export_skr();
  export_snetsim();
  export_tman();
  export_tupleseq();
  export_udp();
  export_udp2();

  export_overlogCompiler();
}
