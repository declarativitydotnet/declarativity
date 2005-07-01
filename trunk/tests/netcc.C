/*
 * @(#)$Id$
 *
 * Copyright (c) 2004 Intel Corporation. All rights reserved.
 *
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 * DESCRIPTION: Tests for UDP
 *
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */
#include <async.h>
#include <arpc.h>
#include <iostream>

#include "tuple.h"
#include "router.h"
#include "udp.h"
//#include "plsensor.h"
#include "tupleseq.h"
#include "cc.h"
#include "bw.h"
#include "snetsim.h"

#include "print.h"
#include "marshalField.h"
#include "marshal.h"
#include "strToSockaddr.h"
#include "route.h"
#include "pelTransform.h"
#include "unmarshal.h"
#include "unmarshalField.h"
#include "timedPushSource.h"
#include "timedPullSink.h"
#include "slot.h"
#include "loggerI.h"

Router::ConfigurationRef UdpCC_source(Udp *udp, str src, str dest, double drop) {
  // The sending data flow
  Router::ConfigurationRef conf = New refcounted< Router::Configuration >();

  ElementSpecRef data     = conf->addElement(New refcounted< TimedPushSource >("source", .5));
  ElementSpecRef srcAddr  = conf->addElement(New refcounted< PelTransform >(strbuf("src:").cat(src), 
									   strbuf() << "\"" << src << "\""
									   << " pop swallow pop"));
  ElementSpecRef seq      = conf->addElement(New refcounted< Sequence >("Sequence"));
  ElementSpecRef cc       = conf->addElement(New refcounted< CCTx >("Transmit CC", 1, 2048));
  ElementSpecRef marshal  = conf->addElement(New refcounted< Marshal >("marshal data"));
  ElementSpecRef destAddr = conf->addElement(New refcounted< PelTransform >(strbuf("dest:").cat(dest),
                                                                            strbuf() << "\"" << dest << "\""
									    << " pop $0 pop"));
  ElementSpecRef route    = conf->addElement(New refcounted< StrToSockaddr >("Convert dest addr", 0));
  ElementSpecRef udpTx    = conf->addElement(udp->get_tx());

  // The receiving data flow
  ElementSpecRef udpRx     = conf->addElement(udp->get_rx());
  ElementSpecRef unmarshal = conf->addElement(New refcounted< UnmarshalField >("unmarshal ack", 1));
  ElementSpecRef unbox     = conf->addElement(New refcounted< PelTransform >("unRoute", "$1 unboxPop"));

  // The local data flow
  conf->hookUp(data, 0, srcAddr, 0);
  conf->hookUp(srcAddr, 0, seq, 0);
  conf->hookUp(seq, 0, cc, 0);
  conf->hookUp(cc, 0, marshal, 0);
  conf->hookUp(marshal, 0, destAddr, 0);
  conf->hookUp(destAddr, 0, route, 0);
  conf->hookUp(route, 0, udpTx, 0);

  // The local ack flow
  conf->hookUp(udpRx, 0, unmarshal, 0);
  conf->hookUp(unmarshal, 0, unbox, 0);
  conf->hookUp(unbox, 0, cc, 1);

  return conf;
}

Router::ConfigurationRef UdpCC_sink(Udp *udp, double drop) {
  Router::ConfigurationRef conf = New refcounted< Router::Configuration >();

  // The remote data elements
  ElementSpecRef udpRx     = conf->addElement(udp->get_rx());
  ElementSpecRef bw        = conf->addElement(New refcounted< Bandwidth >());
  ElementSpecRef unroute   = conf->addElement(New refcounted< PelTransform >("unRoute", "$1 pop"));
  ElementSpecRef unmarshal = conf->addElement(New refcounted< Unmarshal >("unmarshal"));
  ElementSpecRef cc        = conf->addElement(New refcounted< CCRx >("CC Receive", 2048));
  ElementSpecRef unpack    = conf->addElement(New refcounted< PelTransform >("unpackage", "$2 pop"));
  ElementSpecRef slot      = conf->addElement(New refcounted< Slot >("slotRx"));
  ElementSpecRef sinkP     = conf->addElement(New refcounted< Print >("SINK PRINT"));
  ElementSpecRef sink      = conf->addElement(New refcounted< TimedPullSink >("sink", 0));

  // The remote ack elements
  ElementSpecRef udpTx    = conf->addElement(udp->get_tx());
  ElementSpecRef netsim   = conf->addElement(New refcounted< SimpleNetSim >("Simple Net Sim (Sender)", 10, 1000, drop));
  ElementSpecRef route    = conf->addElement(New refcounted< StrToSockaddr >("Convert dest addr", 0));
  ElementSpecRef marshal  = conf->addElement(New refcounted< MarshalField >("marshal ack", 1));
  ElementSpecRef destAddr = conf->addElement(New refcounted< PelTransform >("RESPONSE ADDRESS",
  									    "$0 pop swallow pop"));


  // PACKET RECEIVE DATA FLOW
  conf->hookUp(udpRx, 0, bw, 0);
  conf->hookUp(bw, 0, unroute, 0);
  conf->hookUp(unroute, 0, unmarshal, 0);
  conf->hookUp(unmarshal, 0, cc, 0);
  conf->hookUp(cc, 0, unpack, 0);
  conf->hookUp(unpack, 0, slot, 0);
  conf->hookUp(slot, 0, sinkP, 0);
  conf->hookUp(sinkP, 0, sink, 0);

  // ACK DATA FLOW
  conf->hookUp(cc, 1, destAddr, 0);
  conf->hookUp(destAddr, 0, marshal, 0);
  conf->hookUp(marshal, 0, route, 0);
  conf->hookUp(route, 0, netsim, 0);
  conf->hookUp(netsim, 0, udpTx, 0);

  return conf;
}

void testUdpCC(Router::ConfigurationRef conf)
{
  RouterRef router = New refcounted< Router >(conf, LoggerI::WARN);
  if (router->initialize(router) == 0) {
    std::cout << "Correctly initialized.\n";
  } else {
    std::cout << "** Failed to initialize correct spec\n";
  }

  // Activate the router
  router->activate();

  // Run the router
  amain();
}

int main(int argc, char **argv)
{
   str    type = str(argv[1]);
   double drop = 0.;
   if (argc == 3) drop = atof(argv[2]);

   if (type == "source") {
       Udp *src = new Udp("SOURCE", 10000);
       testUdpCC(UdpCC_source(src, "127.0.0.1:10000", "127.0.0.1:10001", drop));
   }
   else if (type == "sink") {
       Udp *sink = new Udp("SINK", 10001);
       testUdpCC(UdpCC_sink(sink, drop));
   }

  return 0;
}
  

/*
 * End of file 
 */
