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

Router::ConfigurationPtr UdpCC_source(Udp *udp, string src, string dest, double drop) {
  // The sending data flow
  Router::ConfigurationPtr conf(new Router::Configuration());

  ElementSpecPtr data     = conf->addElement(ElementPtr(new TimedPushSource("source", .01)));
  ElementSpecPtr srcAddr  = conf->addElement(ElementPtr(new PelTransform(string("src:").append(src), 
									   "\"" + src + "\""
									   + " pop swallow pop")));
  ElementSpecPtr seq      = conf->addElement(ElementPtr(new Sequence("Sequence")));
  ElementSpecPtr cc       = conf->addElement(ElementPtr(new CCTx("Transmit CC", 1, 2048)));
  ElementSpecPtr marshal  = conf->addElement(ElementPtr(new Marshal("marshal data")));
  ElementSpecPtr destAddr = conf->addElement(ElementPtr(new PelTransform("dest:"+dest,
                                                                         "\"" + dest + "\""
									    + " pop $0 pop")));
  ElementSpecPtr route    = conf->addElement(ElementPtr(new StrToSockaddr("Convert dest addr", 0)));
  ElementSpecPtr netsim   = conf->addElement(ElementPtr(new SimpleNetSim("Simple Net Sim (Sender)", 
									    10, 100, drop)));
  ElementSpecPtr udpTx    = conf->addElement(udp->get_tx());

  // The receiving data flow
  ElementSpecPtr udpRx     = conf->addElement(udp->get_rx());
  ElementSpecPtr unmarshal = conf->addElement(ElementPtr(new UnmarshalField("unmarshal ack", 1)));
  ElementSpecPtr unbox     = conf->addElement(ElementPtr(new PelTransform("unRoute", "$1 unboxPop")));

  // The local data flow
  conf->hookUp(data, 0, srcAddr, 0);
  conf->hookUp(srcAddr, 0, seq, 0);
  conf->hookUp(seq, 0, cc, 0);
  conf->hookUp(cc, 0, marshal, 0);
  conf->hookUp(marshal, 0, destAddr, 0);
  conf->hookUp(destAddr, 0, route, 0);
  conf->hookUp(route, 0, netsim, 0);
  conf->hookUp(netsim, 0, udpTx, 0);

  // The local ack flow
  conf->hookUp(udpRx, 0, unmarshal, 0);
  conf->hookUp(unmarshal, 0, unbox, 0);
  conf->hookUp(unbox, 0, cc, 1);

  return conf;
}

Router::ConfigurationPtr UdpCC_sink(Udp *udp, double drop) {
  Router::ConfigurationPtr conf(new Router::Configuration());

  // The remote data elements
  ElementSpecPtr udpRx     = conf->addElement(udp->get_rx());
  ElementSpecPtr bw        = conf->addElement(ElementPtr(new Bandwidth()));
  ElementSpecPtr unroute   = conf->addElement(ElementPtr(new PelTransform("unRoute", "$1 pop")));
  ElementSpecPtr unmarshal = conf->addElement(ElementPtr(new Unmarshal("unmarshal")));
  ElementSpecPtr cc        = conf->addElement(ElementPtr(new CCRx("CC Receive", 2048)));
  ElementSpecPtr unpack    = conf->addElement(ElementPtr(new PelTransform("unpackage", "$2 pop")));
  ElementSpecPtr slot      = conf->addElement(ElementPtr(new Slot("slotRx")));
  ElementSpecPtr sinkP     = conf->addElement(ElementPtr(new Print("SINK PRINT")));
  ElementSpecPtr sink      = conf->addElement(ElementPtr(new TimedPullSink("sink", 0)));

  // The remote ack elements
  ElementSpecPtr udpTx    = conf->addElement(udp->get_tx());
  ElementSpecPtr netsim   = conf->addElement(ElementPtr(new SimpleNetSim("Simple Net Sim (Sender)", 
									    10, 100, drop)));
  ElementSpecPtr route    = conf->addElement(ElementPtr(new StrToSockaddr("Convert src addr", 0)));
  ElementSpecPtr marshal  = conf->addElement(ElementPtr(new MarshalField("marshal ack", 1)));
  ElementSpecPtr ackP     = conf->addElement(ElementPtr(new Print("ACK PRINT")));
  ElementSpecPtr destAddr = conf->addElement(ElementPtr(new PelTransform("RESPONSE ADDRESS",
  									    "$0 pop swallow pop")));


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
  conf->hookUp(destAddr, 0, ackP, 0);
  conf->hookUp(ackP, 0, marshal, 0);
  conf->hookUp(marshal, 0, route, 0);
  conf->hookUp(route, 0, netsim, 0);
  conf->hookUp(netsim, 0, udpTx, 0);

  return conf;
}

void testUdpCC(Router::ConfigurationPtr conf)
{
  RouterPtr router(new Router(conf, LoggerI::WARN));
  if (router->initialize(router) == 0) {
    std::cout << "Correctly initialized.\n";
  } else {
    std::cout << "** Failed to initialize correct spec\n";
  }

  // Activate the router
  router->activate();

  // Run the router
  eventLoop();
}

int main(int argc, char **argv)
{
  if (argc < 3) {
    std::cout << "Usage: netcc {(source <port> <src_addr> <dest_addr:port>) | sink <port>} [<drop_probability>]\n";
    exit(0);
  }

  string    type = string(argv[1]);
  int    port = atoi(argv[2]);
  double drop = 0.;

  eventLoopInitialize();
  if (type == "source") {
      Udp *src = new Udp("SOURCE", port);
      ostringstream oss;
      oss << string(argv[3]) << ":" << port;
      if (argc == 6) drop = atof(argv[5]);
      testUdpCC(UdpCC_source(src, oss.str(), string(argv[4]), drop));
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
