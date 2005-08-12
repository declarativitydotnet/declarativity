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
#include "cct.h"
#include "ccr.h"
#include "bw.h"
#include "snetsim.h"
#include "rdelivery.h"

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
#include "queue.h"
#include "discard.h"
#include "mux.h"

Router::ConfigurationRef UdpCC_source(Udp *udp, str src, str dest, double drop) {
  // The sending data flow
  Router::ConfigurationRef conf = New refcounted< Router::Configuration >();

  ElementSpecRef data     = conf->addElement(New refcounted< TimedPushSource >("source", .01));
  ElementSpecRef dqueue   = conf->addElement(New refcounted< Queue >("Source Data Q", 1000));
  ElementSpecRef seq      = conf->addElement(New refcounted< Sequence >("Sequence", src, 1));
  ElementSpecRef retry    = conf->addElement(New refcounted< RDelivery >("Retry"));
  ElementSpecRef retryMux = conf->addElement(New refcounted< Mux >("Retry Mux", 2));
  ElementSpecRef cct      = conf->addElement(New refcounted< CCT >("Transmit CC", 1, 2048));
  ElementSpecRef destAddr = conf->addElement(New refcounted< PelTransform >(strbuf("DEST: ").cat(dest),
                                                                            strbuf() << "\"" << dest << "\""
									    << " pop swallow pop"));
  ElementSpecRef marshal  = conf->addElement(New refcounted< MarshalField >("marshal data", 1));
  ElementSpecRef route    = conf->addElement(New refcounted< StrToSockaddr >("Convert dest addr", 0));
  ElementSpecRef netsim   = conf->addElement(New refcounted< SimpleNetSim >("Simple Net Sim (Sender)", 
									    10, 100, drop));
  ElementSpecRef udpTx    = conf->addElement(udp->get_tx());

  // The receiving data flow
  ElementSpecRef udpRx     = conf->addElement(udp->get_rx());
  ElementSpecRef unmarshal = conf->addElement(New refcounted< UnmarshalField >("unmarshal ack", 1));
  ElementSpecRef unbox     = conf->addElement(New refcounted< PelTransform >("unRoute", "$1 unboxPop"));
  ElementSpecRef discard   = conf->addElement(New refcounted< Discard >("DISCARD"));

  // The local data flow
  conf->hookUp(data, 0, dqueue, 0);
  conf->hookUp(dqueue, 0, seq, 0);
  conf->hookUp(seq, 0, retry, 0);
  conf->hookUp(retry, 0, cct, 0);
  conf->hookUp(cct, 0, destAddr, 0);
  conf->hookUp(destAddr, 0, marshal, 0);
  conf->hookUp(marshal, 0, route, 0);
  conf->hookUp(route, 0, netsim, 0);
  conf->hookUp(netsim, 0, udpTx, 0);

  // The local ack flow
  conf->hookUp(udpRx, 0, unmarshal, 0);
  conf->hookUp(unmarshal, 0, unbox, 0);
  conf->hookUp(unbox, 0, cct, 1);
  conf->hookUp(cct, 1, retry, 1);
  conf->hookUp(retry, 1, retryMux, 0);
  conf->hookUp(retry, 2, retryMux, 1);
  conf->hookUp(retryMux, 0, discard, 0);

  return conf;
}

Router::ConfigurationRef UdpCC_sink(Udp *udp, double drop) {
  Router::ConfigurationRef conf = New refcounted< Router::Configuration >();

  // The remote data elements
  ElementSpecRef udpRx     = conf->addElement(udp->get_rx());
  ElementSpecRef bw        = conf->addElement(New refcounted< Bandwidth >());
  ElementSpecRef unmarshal = conf->addElement(New refcounted< UnmarshalField >("unmarshal", 1));
  ElementSpecRef unroute   = conf->addElement(New refcounted< PelTransform >("unRoute", "$1 unboxPop"));
  ElementSpecRef printS    = conf->addElement(New refcounted< Print >("Print Sink"));
  ElementSpecRef ccr       = conf->addElement(New refcounted< CCR >("CC Receive", 2048));
  ElementSpecRef discard   = conf->addElement(New refcounted< Discard >("DISCARD"));

  // The remote ack elements
  ElementSpecRef udpTx    = conf->addElement(udp->get_tx());
  ElementSpecRef netsim   = conf->addElement(New refcounted< SimpleNetSim >("Simple Net Sim (Sender)", 
									    10, 100, drop));
  ElementSpecRef route    = conf->addElement(New refcounted< StrToSockaddr >("Convert src addr", 0));
  ElementSpecRef marshal  = conf->addElement(New refcounted< MarshalField >("marshal ack", 1));
  ElementSpecRef destAddr = conf->addElement(New refcounted< PelTransform >("RESPONSE ADDRESS",
  									    "$0 pop swallow pop"));


  // PACKET RECEIVE DATA FLOW
  conf->hookUp(udpRx, 0, bw, 0);
  conf->hookUp(bw, 0, unmarshal, 0);
  conf->hookUp(unmarshal, 0, unroute, 0);
  conf->hookUp(unroute, 0, printS, 0);
  conf->hookUp(printS, 0, ccr, 0);
  conf->hookUp(ccr, 0, discard, 0);

  // ACK DATA FLOW
  conf->hookUp(ccr, 1, destAddr, 0);
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
  if (argc < 3) {
    std::cout << "Usage: netcc {(source <port> <src_addr> <dest_addr:port>) | sink <port>} [<drop_probability>]\n";
    exit(0);
  }

  str    type = str(argv[1]);
  int    port = atoi(argv[2]);
  double drop = 0.;

  if (type == "source") {
      Udp *src = new Udp("SOURCE", port);
      if (argc == 6) drop = atof(argv[5]);
      testUdpCC(UdpCC_source(src, strbuf() << str(argv[3]) << ":" << port, 
			     str(argv[4]), drop));
  }
  else if (type == "sink") {
      Udp *sink = new Udp("SINK", port);
      if (argc == 4) drop = atof(argv[3]);
      testUdpCC(UdpCC_sink(sink, drop));
  }

  return 0;
}
  

/*
 * End of file 
 */
