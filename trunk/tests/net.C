/*
 * @(#)$Id$
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
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "tuple.h"
#include "plumber.h"
#include "udp.h"
#include "plsensor.h"

#include "print.h"
#include "marshal.h"
#include "route.h"
#include "pelTransform.h"
#include "unmarshal.h"
#include "timedPushSource.h"
#include "timedPullSink.h"
#include "slot.h"
#include "loggerI.h"

/** Test an element chain that packages a tuple and sends it over
    Udp. */
void testUdpTx()
{
  eventLoopInitialize();
  std::cout << "\nCHECK UDP Transmit\n";

  // The udp objects
  Udp udpOut("9999", 9999);
  Udp udpIn("10000", 10000);

  // The destination address
  string destinationAddr = "127.0.0.1";
  struct sockaddr_in addr;
  bzero(&addr, sizeof(addr));
  addr.sin_port = htons(10000);
  inet_pton(AF_INET, destinationAddr.c_str(),
            &addr.sin_addr);
  FdbufPtr addressUio(new Fdbuf());
  addressUio->push_bytes((char*)&addr, sizeof(addr));


  // The sending data flow

  PlumberPtr plumber(new Plumber());
  Plumber::DataflowPtr conf = plumber->new_dataflow("test");
  ElementSpecPtr sourceS =
    conf->addElement(ElementPtr(new TimedPushSource("source", .5)));
  ElementSpecPtr sourcePrintS =
    conf->addElement(ElementPtr(new Print("AfterSource")));
  ElementSpecPtr marshalS =
    conf->addElement(ElementPtr(new Marshal("marshal")));
  ElementSpecPtr marshalPrintS =
    conf->addElement(ElementPtr(new Print("Marshalled")));
  ElementSpecPtr routeS =
    conf->addElement(ElementPtr(new Route("plumber", addressUio)));
  ElementSpecPtr routePrintS = conf->addElement(ElementPtr(new Print("Routed")));
  ElementSpecPtr udpTxS = conf->addElement(udpOut.get_tx());
  ElementSpecPtr slotTxS = conf->addElement(ElementPtr(new Slot("slotTx")));

  conf->hookUp(sourceS, 0, sourcePrintS, 0);
  conf->hookUp(sourcePrintS, 0, marshalS, 0);
  conf->hookUp(marshalS, 0, marshalPrintS, 0);
  conf->hookUp(marshalPrintS, 0, routeS, 0);
  conf->hookUp(routeS, 0, routePrintS, 0);
  conf->hookUp(routePrintS, 0, slotTxS, 0);
  conf->hookUp(slotTxS, 0, udpTxS, 0);



  // The receiving data flow
  ElementSpecPtr udpRxS = conf->addElement(udpIn.get_rx());
  ElementSpecPtr rxPrintS = conf->addElement(ElementPtr(new Print("Received")));
  ElementSpecPtr unrouteS =
    conf->addElement(ElementPtr(new PelTransform("unRoute", "$1 pop")));
  ElementSpecPtr unroutePrintS = conf->addElement(ElementPtr(new Print("DropAddress")));
  ElementSpecPtr unmarshalS =
    conf->addElement(ElementPtr(new Unmarshal("unmarshal")));
  ElementSpecPtr sinkPrintS = conf->addElement(ElementPtr(new Print("BeforeSink")));
  ElementSpecPtr slotRxS = conf->addElement(ElementPtr(new Slot("slotRx")));
  ElementSpecPtr sinkS =
    conf->addElement(ElementPtr(new TimedPullSink("sink", 0)));
  conf->hookUp(udpRxS, 0, rxPrintS, 0);
  conf->hookUp(rxPrintS, 0, unrouteS, 0);
  conf->hookUp(unrouteS, 0, unroutePrintS, 0);
  conf->hookUp(unroutePrintS, 0, unmarshalS, 0);
  conf->hookUp(unmarshalS, 0, sinkPrintS, 0);
  conf->hookUp(sinkPrintS, 0, slotRxS, 0);
  conf->hookUp(slotRxS, 0, sinkS, 0);

  if (plumber->install(conf) == 0) {
    std::cout << "Correctly initialized.\n";
  } else {
    std::cout << "** Failed to initialize correct spec\n";
  }

  // Run the plumber
  eventLoop();
}

/** Test the Rx part of the Udp element. */
void testPLSensor()
{
  eventLoopInitialize();
  std::cout << "\nCHECK PL SENSOR\n";

  boost::shared_ptr<PlSensor> pl(new PlSensor("Sensor", (uint16_t)80,"/", 5));
  PlumberPtr plumber(new Plumber());
  Plumber::DataflowPtr conf = plumber->new_dataflow("test");
  ElementSpecPtr plSpec = conf->addElement(pl);
  ElementSpecPtr printSpec = conf->addElement(ElementPtr(new Print("PRINT SPEC")));
  conf->hookUp(plSpec,0,printSpec,0);

  // Create the plumber and check it statically
  if (plumber->install(conf) == 0) {
    std::cout << "Correctly initialized PlSensor to push print spec.\n";
  } else {
    std::cout << "** Failed to initialize correct spec\n";
  }

  // Run the plumber
  eventLoop();
}

int main(int argc, char **argv)
{
  std::cout << "\nNET\n";

  testUdpTx();
  testPLSensor();

  return 0;
}
  

/*
 * End of file 
 */
