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

#include "print.h"
#include "marshal.h"
#include "route.h"
#include "pelTransform.h"
#include "unmarshal.h"
#include "timedPushSource.h"
#include "timedPullSink.h"
#include "slot.h"
#include "loggerI.h"

#if 0 
/** Test an element chain that packages a tuple and sends it over
    Udp. */
void testUdpTx()
{
  std::cout << "\nCHECK UDP Transmit\n";

  // The udp objects
  Udp udpOut("9999", 9999);
  Udp udpIn("10000", 10000);

  // The destination address
  str destinationAddr = "127.0.0.1";
  struct sockaddr_in addr;
  bzero(&addr, sizeof(addr));
  addr.sin_port = htons(10000);
  inet_pton(AF_INET, destinationAddr.cstr(),
            &addr.sin_addr);
  ref< suio > addressUio = New refcounted< suio >();
  addressUio->copy(&addr, sizeof(addr));


  // The sending data flow

  Router::ConfigurationRef conf = New refcounted< Router::Configuration >();
  ElementSpecRef sourceS =
    conf->addElement(New refcounted< TimedPushSource >("source", .5));
  ElementSpecRef sourcePrintS =
    conf->addElement(New refcounted< Print >("AfterSource"));
  ElementSpecRef marshalS =
    conf->addElement(New refcounted< Marshal >("marshal"));
  ElementSpecRef marshalPrintS =
    conf->addElement(New refcounted< Print >("Marshalled"));
  ElementSpecRef routeS =
    conf->addElement(New refcounted< Route >("router", addressUio));
  ElementSpecRef routePrintS = conf->addElement(New refcounted< Print >("Routed"));
  ElementSpecRef udpTxS = conf->addElement(udpOut.get_tx());
  ElementSpecRef slotTxS = conf->addElement(New refcounted< Slot >("slotTx"));

  conf->hookUp(sourceS, 0, sourcePrintS, 0);
  conf->hookUp(sourcePrintS, 0, marshalS, 0);
  conf->hookUp(marshalS, 0, marshalPrintS, 0);
  conf->hookUp(marshalPrintS, 0, routeS, 0);
  conf->hookUp(routeS, 0, routePrintS, 0);
  conf->hookUp(routePrintS, 0, slotTxS, 0);
  conf->hookUp(slotTxS, 0, udpTxS, 0);



  // The receiving data flow
  ElementSpecRef udpRxS = conf->addElement(udpIn.get_rx());
  ElementSpecRef rxPrintS = conf->addElement(New refcounted< Print >("Received"));
  ElementSpecRef unrouteS =
    conf->addElement(New refcounted< PelTransform >("unRoute", "$1 pop"));
  ElementSpecRef unroutePrintS = conf->addElement(New refcounted< Print >("DropAddress"));
  ElementSpecRef unmarshalS =
    conf->addElement(New refcounted< Unmarshal >("unmarshal"));
  ElementSpecRef sinkPrintS = conf->addElement(New refcounted< Print >("BeforeSink"));
  ElementSpecRef slotRxS = conf->addElement(New refcounted< Slot >("slotRx"));
  ElementSpecRef sinkS =
    conf->addElement(New refcounted< TimedPullSink >("sink", 0));
  conf->hookUp(udpRxS, 0, rxPrintS, 0);
  conf->hookUp(rxPrintS, 0, unrouteS, 0);
  conf->hookUp(unrouteS, 0, unroutePrintS, 0);
  conf->hookUp(unroutePrintS, 0, unmarshalS, 0);
  conf->hookUp(unmarshalS, 0, sinkPrintS, 0);
  conf->hookUp(sinkPrintS, 0, slotRxS, 0);
  conf->hookUp(slotRxS, 0, sinkS, 0);

  RouterRef router = New refcounted< Router >(conf);
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

/** Test the Rx part of the Udp element. */
void testPLSensor()
{
  std::cout << "\nCHECK PL SENSOR\n";

  ref<PlSensor> pl = New refcounted<PlSensor>((uint16_t)80,"/", 5);
  Router::ConfigurationRef conf = New refcounted< Router::Configuration >();
  ElementSpecRef plSpec = conf->addElement(pl);
  ElementSpecRef printSpec = conf->addElement(New refcounted<PushPrint>());
  conf->hookUp(plSpec,0,printSpec,0);

  // Create the router and check it statically
  RouterRef router = New refcounted< Router >(conf);
  if (router->initialize(router) == 0) {
    std::cout << "Correctly initialized PlSensor to push print spec.\n";
  } else {
    std::cout << "** Failed to initialize correct spec\n";
  }

  // Activate the router
  router->activate();

  // Run the router
  amain();
}
#endif 

void testUdpCC()
{
  std::cout << "\nCHECK UDP Transmit\n";

  // The udp data objects
  Udp udpDataOut("10000", 10000);	// Data Channel out
  Udp udpDataIn("10001", 10001);	// Data Channel in

  // The udp ack objects
  Udp udpAckIn("10002", 10002);		// Ack Channel in
  Udp udpAckOut("10003", 10003); 	// Ack CHannel out

  CC cc(1, 16);				// CC element

  str ipAddr = "127.0.0.1";
  struct sockaddr_in addr;

  // The local data address
  bzero(&addr, sizeof(addr));
  addr.sin_port = htons(10001);
  inet_pton(AF_INET, ipAddr.cstr(), &addr.sin_addr);
  ref< suio > localAddr_data = New refcounted< suio >();
  localAddr_data->copy(&addr, sizeof(addr));

  // The remote ack address
  bzero(&addr, sizeof(addr));
  addr.sin_port = htons(10002);
  inet_pton(AF_INET, ipAddr.cstr(), &addr.sin_addr);
  ref< suio > remoteAddr_ack = New refcounted< suio >();
  remoteAddr_ack->copy(&addr, sizeof(addr));


  // The sending data flow
  Router::ConfigurationRef conf = New refcounted< Router::Configuration >();

  ElementSpecRef sourceL           = conf->addElement(New refcounted< TimedPushSource >("source", .5));
  ElementSpecRef sourcePrintL      = conf->addElement(New refcounted< Print >("AfterSource"));
  ElementSpecRef seqL              = conf->addElement(New refcounted< TupleSeq::Package >("TupleSeq::Package"));
  ElementSpecRef ccL               = conf->addElement(cc.get_tx());
  ElementSpecRef ccPrintL          = conf->addElement(New refcounted< Print >("AfterCC"));
  ElementSpecRef marshalDataL      = conf->addElement(New refcounted< Marshal >("marshal data"));
  ElementSpecRef routeL            = conf->addElement(New refcounted< Route >("router src", localAddr_data));
  ElementSpecRef udpTxL            = conf->addElement(udpDataOut.get_tx());

  ElementSpecRef udpRxL            = conf->addElement(udpAckIn.get_rx());
  ElementSpecRef unmarshalAckL     = conf->addElement(New refcounted< Unmarshal >("unmarshal ack"));
  ElementSpecRef unrouteL          = conf->addElement(New refcounted< PelTransform >("unRoute", "$1 pop"));

  // The local data flow
  conf->hookUp(sourceL, 0, sourcePrintL, 0);
  conf->hookUp(sourcePrintL, 0, seqL, 0);
  conf->hookUp(seqL, 0, ccL, 0);
  conf->hookUp(ccL, 0, ccPrintL, 0);
  conf->hookUp(ccPrintL, 0, marshalDataL, 0);
  conf->hookUp(marshalDataL, 0, routeL, 0);
  conf->hookUp(routeL, 0, udpTxL, 0);

  // The local ack flow
  conf->hookUp(udpRxL, 0, unrouteL, 0);
  conf->hookUp(unrouteL, 0, unmarshalAckL, 0);
  conf->hookUp(unmarshalAckL, 0, ccL, 1);

  // The remote data flow
  ElementSpecRef udpRxR        = conf->addElement(udpDataIn.get_rx());
  ElementSpecRef unrouteR      = conf->addElement(New refcounted< PelTransform >("unRoute", "$1 pop"));
  ElementSpecRef unmarshalR    = conf->addElement(New refcounted< Unmarshal >("unmarshal"));
  ElementSpecRef sinkPrintSeq  = conf->addElement(New refcounted< Print >("Before CC"));
  ElementSpecRef ccR           = conf->addElement(cc.get_rx());
  ElementSpecRef sinkSeq       = conf->addElement(New refcounted< TupleSeq::Unpackage >());
  ElementSpecRef sinkPrintR    = conf->addElement(New refcounted< Print >("After CC"));
  ElementSpecRef slotRxR       = conf->addElement(New refcounted< Slot >("slotRx"));
  ElementSpecRef sinkR         = conf->addElement(New refcounted< TimedPullSink >("sink", 0));

  // The remote ack flow
  ElementSpecRef udpTxR        = conf->addElement(udpAckOut.get_tx());
  ElementSpecRef marshalAckR   = conf->addElement(New refcounted< Marshal >("marshal ack"));
  ElementSpecRef routeR        = conf->addElement(New refcounted< Route >("router dest", remoteAddr_ack));
  ElementSpecRef printAck      = conf->addElement(New refcounted< Print >("Print Ack"));


  conf->hookUp(udpRxR, 0, unrouteR, 0);
  conf->hookUp(unrouteR, 0, unmarshalR, 0);
  conf->hookUp(unmarshalR, 0, sinkPrintSeq, 0);
  conf->hookUp(sinkPrintSeq, 0, ccR, 0);
  conf->hookUp(ccR, 0, sinkSeq, 0);
  conf->hookUp(sinkSeq, 0, sinkPrintR, 0);
  conf->hookUp(sinkPrintR, 0, slotRxR, 0);
  conf->hookUp(slotRxR, 0, sinkR, 0);


  conf->hookUp(ccR, 1, printAck, 0);
  conf->hookUp(printAck, 0, marshalAckR, 0);
  conf->hookUp(marshalAckR, 0, routeR, 0);
  conf->hookUp(routeR, 0, udpTxR, 0);

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
  std::cout << "\nNET\n";

  // testUdpTx();
  //  testPLSensor();
  testUdpCC();

  return 0;
}
  

/*
 * End of file 
 */
