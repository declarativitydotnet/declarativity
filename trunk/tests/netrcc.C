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
#include "tupleseq.h"
#include "rcct.h"
#include "rccr.h"
#include "bw.h"
#include "snetsim.h"
#include "rdelivery.h"
#include "roundRobin.h"

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
#include "loggerI.h"
#include "discard.h"

Router::ConfigurationPtr UdpCC_source(Udp *udp, string src, string dest, double drop) {
  // The sending data flow
  Router::ConfigurationPtr conf(new Router::Configuration());

  ElementSpecPtr data     = conf->addElement(ElementPtr(new TimedPushSource("source", .01)));
  ElementSpecPtr dataq    = conf->addElement(ElementPtr(new Queue("Data Q", 100)));
  ElementSpecPtr rr       = conf->addElement(ElementPtr(new RoundRobin("RR", 2)));
  ElementSpecPtr seq      = conf->addElement(ElementPtr(new Sequence("Sequence", src, 1)));
  ElementSpecPtr retry    = conf->addElement(ElementPtr(new RDelivery("Retry", false)));
  ElementSpecPtr rcct     = conf->addElement(ElementPtr(new RateCCT("RateCCT")));
  ElementSpecPtr destAddr = conf->addElement(ElementPtr(new PelTransform("dest:"+dest,
                                             "\"" + dest + "\"" + " pop swallow pop")));
  ElementSpecPtr marshal  = conf->addElement(ElementPtr(new MarshalField("marshal data", 1)));
  ElementSpecPtr route    = conf->addElement(ElementPtr(new StrToSockaddr("sock2addr", 0)));
  ElementSpecPtr netsim   = conf->addElement(ElementPtr(new SimpleNetSim("Net Sim (Sender)", 
									    90, 100, drop)));
  ElementSpecPtr udpTx    = conf->addElement(udp->get_tx());

  // The receiving data flow
  ElementSpecPtr udpRx     = conf->addElement(udp->get_rx());
  ElementSpecPtr unmarshal = conf->addElement(ElementPtr(new UnmarshalField("ack", 1)));
  ElementSpecPtr unbox     = conf->addElement(ElementPtr(new PelTransform("unRoute", 
									     "$1 unboxPop")));
  ElementSpecPtr retryP   = conf->addElement(ElementPtr(new Print("RETRY PRINT")));
  ElementSpecPtr strip    = conf->addElement(ElementPtr(new PelTransform("strip", "$1 unboxPop")));
  ElementSpecPtr retryQ   = conf->addElement(ElementPtr(new Queue("RETRY Q", 100)));
  ElementSpecPtr retryQP  = conf->addElement(ElementPtr(new Print("RETRY AFTER QUEUE PRINT")));
  ElementSpecPtr discard  = conf->addElement(ElementPtr(new Discard("Discard retry data")));

  // The local data flow
  conf->hookUp(data, 0, dataq, 0);
  conf->hookUp(dataq, 0, rr, 0);
  conf->hookUp(rr, 0, seq, 0);
  conf->hookUp(seq, 0, retry, 0);
  conf->hookUp(retry, 0, rcct, 0);
  conf->hookUp(rcct, 0, destAddr, 0);
  conf->hookUp(destAddr, 0, marshal, 0);
  conf->hookUp(marshal, 0, route, 0);
  conf->hookUp(route, 0, netsim, 0);
  conf->hookUp(netsim, 0, udpTx, 0);

  // The local ack flow
  conf->hookUp(udpRx, 0, unmarshal, 0);
  conf->hookUp(unmarshal, 0, unbox, 0);
  conf->hookUp(unbox, 0, rcct, 1);
  conf->hookUp(rcct, 1, retry, 1);
  conf->hookUp(retry, 1, discard, 0);
  conf->hookUp(retry, 2, strip, 0);
  conf->hookUp(strip, 0, retryP, 0);
  conf->hookUp(retryP, 0, retryQ, 0);
  conf->hookUp(retryQ, 0, retryQP, 0);
  conf->hookUp(retryQP, 0, rr, 1);

  return conf;
}

Router::ConfigurationPtr UdpCC_sink(Udp *udp, double drop) {
  Router::ConfigurationPtr conf(new Router::Configuration());

  // The remote data elements
  ElementSpecPtr udpRx     = conf->addElement(udp->get_rx());
  ElementSpecPtr bw        = conf->addElement(ElementPtr(new Bandwidth()));
  ElementSpecPtr unmarshal = conf->addElement(ElementPtr(new UnmarshalField("unmarshal", 1)));
  ElementSpecPtr unroute   = conf->addElement(ElementPtr(new PelTransform("unRoute", "$1 unboxPop")));
  ElementSpecPtr rccr      = conf->addElement(ElementPtr(new RateCCR("RCC Receive")));
  // ElementSpecPtr sinkP     = conf->addElement(ElementPtr(new Print("SINK PRINT")));
  ElementSpecPtr discard   = conf->addElement(ElementPtr(new Discard("Discard sink")));

  // The remote ack elements
  ElementSpecPtr udpTx    = conf->addElement(udp->get_tx());
  ElementSpecPtr netsim   = conf->addElement(ElementPtr(new SimpleNetSim("Net Sim (Sink)", 
									    90, 100, drop)));
  ElementSpecPtr route    = conf->addElement(ElementPtr(new StrToSockaddr("convert addr", 0)));
  ElementSpecPtr marshal  = conf->addElement(ElementPtr(new MarshalField("marshal ack", 1)));
  ElementSpecPtr ackP     = conf->addElement(ElementPtr(new Print("ACK PRINT")));
  ElementSpecPtr destAddr = conf->addElement(ElementPtr(new PelTransform(
			      		     "RESPONSE ADDRESS", "$0 pop swallow pop")));


  // PACKET RECEIVE DATA FLOW
  conf->hookUp(udpRx, 0, bw, 0);
  conf->hookUp(bw, 0, unmarshal, 0);
  conf->hookUp(unmarshal, 0, unroute, 0);
  conf->hookUp(unroute, 0, rccr, 0);
  conf->hookUp(rccr, 0, discard, 0);
  // conf->hookUp(sinkP, 0, discard, 0);

  // ACK DATA FLOW
  conf->hookUp(rccr, 1, destAddr, 0);
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
    std::cout << "Usage: netrcc {(source <port> <src_addr> <dest_addr:port>) | sink <port>} [<drop_probability>]\n";
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
