/*
 * @(#)$Id$
 *
 *
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * Or
 * UC Berkeley EECS Computer Science Division, 387 Soda Hall #1776, 
 * Berkeley, CA,  94707. Attention: P2 Group.
 * 
 * DESCRIPTION: Tests for UDP
 *
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */
#include <iostream>

#include "tuple.h"
#include "plumber.h"
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

Plumber::ConfigurationPtr UdpCC_source(Udp *udp, string src, string dest, double drop) {
  // The sending data flow
  Plumber::ConfigurationPtr conf(new Plumber::Configuration());

  ElementSpecPtr data     = conf->addElement(ElementPtr(new TimedPushSource("source", .01)));
  ElementSpecPtr dqueue   = conf->addElement(ElementPtr(new Queue("Source Data Q", 1000)));
  ElementSpecPtr seq      = conf->addElement(ElementPtr(new Sequence("Sequence", src, 1)));
  ElementSpecPtr retry    = conf->addElement(ElementPtr(new RDelivery("Retry")));
  ElementSpecPtr retryMux = conf->addElement(ElementPtr(new Mux("Retry Mux", 2)));
  ElementSpecPtr cct      = conf->addElement(ElementPtr(new CCT("Transmit CC", 1, 2048)));
  ElementSpecPtr destAddr = conf->addElement(ElementPtr(new PelTransform(string("DEST: ").append(dest),
                                                                            "\"" + dest + "\""
									    + " pop swallow pop")));
  ElementSpecPtr marshal  = conf->addElement(ElementPtr(new MarshalField("marshal data", 1)));
  ElementSpecPtr route    = conf->addElement(ElementPtr(new StrToSockaddr("Convert dest addr", 0)));
  ElementSpecPtr netsim   = conf->addElement(ElementPtr(new SimpleNetSim("Simple Net Sim (Sender)", 
									    10, 100, drop)));
  ElementSpecPtr udpTx    = conf->addElement(udp->get_tx());

  // The receiving data flow
  ElementSpecPtr udpRx     = conf->addElement(udp->get_rx());
  ElementSpecPtr unmarshal = conf->addElement(ElementPtr(new UnmarshalField("unmarshal ack", 1)));
  ElementSpecPtr unbox     = conf->addElement(ElementPtr(new PelTransform("unRoute", "$1 unboxPop")));
  ElementSpecPtr discard   = conf->addElement(ElementPtr(new Discard("DISCARD")));

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

Plumber::ConfigurationPtr UdpCC_sink(Udp *udp, double drop) {
  Plumber::ConfigurationPtr conf(new Plumber::Configuration());

  // The remote data elements
  ElementSpecPtr udpRx     = conf->addElement(udp->get_rx());
  ElementSpecPtr bw        = conf->addElement(ElementPtr(new Bandwidth()));
  ElementSpecPtr unmarshal = conf->addElement(ElementPtr(new UnmarshalField("unmarshal", 1)));
  ElementSpecPtr unroute   = conf->addElement(ElementPtr(new PelTransform("unRoute", "$1 unboxPop")));
  ElementSpecPtr printS    = conf->addElement(ElementPtr(new Print("Print Sink")));
  ElementSpecPtr ccr       = conf->addElement(ElementPtr(new CCR("CC Receive", 2048)));
  ElementSpecPtr discard   = conf->addElement(ElementPtr(new Discard("DISCARD")));

  // The remote ack elements
  ElementSpecPtr udpTx    = conf->addElement(udp->get_tx());
  ElementSpecPtr netsim   = conf->addElement(ElementPtr(new SimpleNetSim("Simple Net Sim (Sender)", 
									    10, 100, drop)));
  ElementSpecPtr route    = conf->addElement(ElementPtr(new StrToSockaddr("Convert src addr", 0)));
  ElementSpecPtr marshal  = conf->addElement(ElementPtr(new MarshalField("marshal ack", 1)));
  ElementSpecPtr destAddr = conf->addElement(ElementPtr(new PelTransform("RESPONSE ADDRESS",
  									    "$0 pop swallow pop")));


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

void testUdpCC(Plumber::ConfigurationPtr conf)
{
  PlumberPtr plumber(new Plumber(conf, LoggerI::WARN));
  if (plumber->initialize(plumber) == 0) {
    std::cout << "Correctly initialized.\n";
  } else {
    std::cout << "** Failed to initialize correct spec\n";
  }

  // Activate the plumber
  plumber->activate();

  // Run the plumber
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
