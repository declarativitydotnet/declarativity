/*
* Copyright (c) 2003 Intel Corporation
* All rights reserved.
*
*/

#include <boost/python/class.hpp>
#include <boost/python/scope.hpp>

#include <boost/python.hpp>
using namespace boost::python;

#include "element.h"

class ElementWrap : public Element, public wrapper<Element>
{
public:
  ElementWrap(std::string n) : Element(n) {};
  ElementWrap(std::string n, int i, int o) : Element(n, i, o) {};

  virtual const char* class_name() const {
    return this->get_override("class_name")();
  };

};

/* P2CORE */
#include "plumber.h"
#include "elementSpec.h"
#include "loggerI.h"
#include "tuple.h"

class LoggerIWrap : public LoggerI, public wrapper<LoggerI>
{
public:
  void log(string classname, string instancename,
           Level severity, int errnum, string explanation) {
    this->get_override("log")(classname, instancename, severity, 
                              errnum, explanation);
  };
};

/* ELEMENTS */
#include "aggregate.h"
#include "aggwrap.h"
#include "csvparser.h"
// #include "ddemux.h"
#include "delete.h"
#include "demux.h"
#include "discard.h"
#include "dupElim.h"
#include "duplicateConservative.h"
#include "duplicate.h"
#include "filter.h"
#include "functorSource.h"
#include "hexdump.h"
#include "insert.h"
// #include "joiner.h"
#include "logger.h"
#include "../elements/marshal.h"
#include "marshalField.h"
#include "mux.h"
#include "noNull.h"
#include "noNullField.h"
#include "pelScan.h"
#include "pelTransform.h"
#include "print.h"
#include "printTime.h"
#include "printWatch.h"
#include "queue.h"
// #include "randomPushSource.h"
#include "roundRobin.h"
#include "scan.h"
#include "slot.h"
#include "strToSockaddr.h"
#include "timedPullPush.h"
#include "timedPullSink.h"
#include "timedPushSource.h"
#include "timestampSource.h"
#include "tupleSource.h"
#include "unboxField.h"
#include "unmarshal.h"
#include "unmarshalField.h"

/* NETWORK ELEMENTS */
#include "bw.h"
#include "ccr.h"
#include "cct.h"
#include "defrag.h"
#include "frag.h"
#include "plsensor.h"
#include "rccr.h"
#include "rcct.h"
#include "rdelivery.h"
#include "skr.h"
#include "snetsim.h"
#include "tman.h"
#include "tupleseq.h"
#include "udp2.h"

/* EVENT LOOP */
#include "loop.h"

BOOST_PYTHON_MODULE(p2python)
{
  #include "p2core/plumber.cpp"
  #include "p2core/elementSpec.cpp"
  #include "p2core/loggerI.cpp"
  #include "p2core/tuple.cpp"

  #include "eventLoop/eventLoop.cpp"
  #include "elements/element.cpp"
  #include "elements/aggregate.cpp"
  #include "elements/aggwrap.cpp"
  #include "elements/csvparser.cpp"
  // #include "elements/ddemux.cpp"
  #include "elements/delete.cpp"
  #include "elements/demux.cpp"
  #include "elements/discard.cpp"
  #include "elements/dupElim.cpp"
  #include "elements/duplicateConservative.cpp"
  #include "elements/duplicate.cpp"
  #include "elements/filter.cpp"
  #include "elements/functorSource.cpp"
  #include "elements/hexdump.cpp"
  #include "elements/insert.cpp"
  // #include "elements/joiner.cpp"
  #include "elements/logger.cpp"
  #include "elements/marshal.cpp"
  #include "elements/marshalField.cpp"
  #include "elements/mux.cpp"
  #include "elements/noNull.cpp"
  #include "elements/noNullField.cpp"
  #include "elements/pelScan.cpp"
  #include "elements/pelTransform.cpp"
  #include "elements/print.cpp"
  #include "elements/printTime.cpp"
  #include "elements/printWatch.cpp"
  #include "elements/queue.cpp"
  // #include "elements/randomPushSource.cpp"
  #include "elements/roundRobin.cpp"
  #include "elements/scan.cpp"
  #include "elements/slot.cpp"
  #include "elements/strToSockaddr.cpp"
  #include "elements/timedPullPush.cpp"
  #include "elements/timedPullSink.cpp"
  #include "elements/timedPushSource.cpp"
  #include "elements/timestampSource.cpp"
  #include "elements/tupleSource.cpp"
  #include "elements/unboxField.cpp"
  #include "elements/unmarshal.cpp"
  #include "elements/unmarshalField.cpp"

  #include "net/bw.cpp"
  #include "net/ccr.cpp"
  #include "net/cct.cpp"
  #include "net/defrag.cpp"
  #include "net/frag.cpp"
  #include "net/plsensor.cpp"
  #include "net/rccr.cpp"
  #include "net/rcct.cpp"
  #include "net/rdelivery.cpp"
  #include "net/skr.cpp"
  #include "net/snetsim.cpp"
  #include "net/tman.cpp"
  #include "net/tupleseq.cpp"
  #include "net/udp2.cpp"
}
