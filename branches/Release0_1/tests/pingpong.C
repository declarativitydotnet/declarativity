// -*- c-basic-offset: 2; related-file-name: "" -*-
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

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#if HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */
#include <iostream>
#include <stdlib.h>
#include <vector>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include "strToSockaddr.h"
#include "tuple.h"
#include "plumber.h"
#include "udp.h"

#include "print.h"
#include "marshal.h"
#include "route.h"
#include "pelTransform.h"
#include "unmarshal.h"
#include "timedPushSource.h"
#include "timedPullSink.h"
#include "ping.h"
#include "pong.h"
#include "slot.h"
#include "val_str.h"
#include "val_uint64.h"
#include "demux.h"
#include "roundRobin.h"
#include "marshalField.h"
#include "unmarshalField.h"
#include "store.h"
#include "timedPullPush.h"
#include "discard.h"

string getLocalHostname()
{
  char* c = (char*) malloc(80 * sizeof(char));
  char *d = (char*) malloc(80 * sizeof(char));
  gethostname(c, 80);
  struct hostent *h = gethostbyname(c);
  sprintf(d, "%s", inet_ntoa(*((struct in_addr *)h->h_addr)));
  string toRet(d);
  free(c);
  free(d);
  return toRet;
}


void testPingPong(int mode, string targetHost, LoggerI::Level level)
{
  eventLoopInitialize();

  // sender mode 0
  // receiver mode 1

  // the local hostname
  string localHostname = getLocalHostname();

  boost::shared_ptr< Store > pingNodeStore(new Store("PingNodes", 2));

  // The sending data flow
  Plumber::ConfigurationPtr conf(new Plumber::Configuration());    

  // to add a demuxer to differentiate ping and pingresponse
  Udp udpOut("9999", 9999); // port of the sender    
  std::vector<TuplePtr> buffer;
  TuplePtr tuple = Tuple::mk();
  tuple->append(Val_Str::mk("Ping"));
  tuple->append(Val_Str::mk(localHostname + ":10000")); // my node
  tuple->append(Val_Str::mk(targetHost + ":10000")); // ping target
  tuple->freeze();  
  if (mode == 0) {
    pingNodeStore->insert(tuple);  
  }

  ElementSpecPtr scanS = conf->addElement(pingNodeStore->mkScan());
  ElementSpecPtr scanPrintS =
    conf->addElement(ElementPtr(new Print("AfterSource")));
  ElementSpecPtr timedPullPushS = conf->addElement(ElementPtr(new TimedPullPush("PushPingNodes", 10)));

  // pass it through the ping element to generate the ping request
  ElementSpecPtr ping = conf->addElement(ElementPtr(new Ping("Ping", 5, 1, 15)));
  ElementSpecPtr pingPrintS = conf->addElement(ElementPtr(new Print("PingPrint")));

  // send the message where the first field is the address
  ElementSpecPtr encapS =
    conf->addElement(ElementPtr(new PelTransform("encapPingRequest",
                                                    "$1 pop \
                                                     $0 ->t $1 append $2 append pop"))); // the rest

  ElementSpecPtr marshalS =
    conf->addElement(ElementPtr(new MarshalField("Marshal", 1)));
  
  ElementSpecPtr routeS =
    conf->addElement(ElementPtr(new StrToSockaddr("Router:", 0)));

  ElementSpecPtr printSPostMarshal = conf->addElement(ElementPtr(new Print("PrintPostMarshal")));

  ElementSpecPtr slotTxS = conf->addElement(ElementPtr(new Slot("slotTx")));
  ElementSpecPtr udpTxS = conf->addElement(udpOut.get_tx());
  ElementSpecPtr muxS = conf->addElement(ElementPtr(new RoundRobin("roundRobin", 2)));
  
  conf->hookUp(scanS, 0, scanPrintS, 0);
  conf->hookUp(scanPrintS, 0, timedPullPushS, 0);
  conf->hookUp(timedPullPushS, 0, ping, 0);
  conf->hookUp(ping, 1, pingPrintS, 0);
  conf->hookUp(pingPrintS, 0, slotTxS, 0);
  conf->hookUp(slotTxS, 0, muxS, 0);
  conf->hookUp(muxS, 0, encapS, 0);
  conf->hookUp(encapS, 0, marshalS, 0);
  conf->hookUp(marshalS, 0, routeS, 0);
  conf->hookUp(routeS, 0, printSPostMarshal, 0);
  conf->hookUp(printSPostMarshal, 0, udpTxS, 0);
  
  // pipe them separately into pong and ping
  Udp udpIn("10000", 10000);  
  ElementSpecPtr udpRxS = conf->addElement(udpIn.get_rx());
  ElementSpecPtr rxPrintS = conf->addElement(ElementPtr(new Print("Received")));
  ElementSpecPtr unmarshalS =
    conf->addElement(ElementPtr(new UnmarshalField("unmarshal", 1)));
  ElementSpecPtr unBoxS =
    conf->addElement(ElementPtr(new PelTransform("UnBox:", "$1 unbox pop pop pop")));

  ElementSpecPtr sinkPrintS = conf->addElement(ElementPtr(new Print("After Unmarshal")));
  boost::shared_ptr< std::vector< ValuePtr > > demuxKeys(new std::vector< ValuePtr >);
  
  ElementSpecPtr discardS = conf->addElement(ElementPtr(new Discard("defaultSink")));
  
  demuxKeys->push_back(ValuePtr(new Val_Str("PingRequest")));
  demuxKeys->push_back(ValuePtr(new Val_Str("PingResponse")));

  ElementSpecPtr demuxS = conf->addElement(ElementPtr(new Demux("demux", demuxKeys)));
  ElementSpecPtr pong = conf->addElement(ElementPtr(new Pong("Pong", 0)));
  ElementSpecPtr pongPrint = conf->addElement(ElementPtr(new Print("PongReply")));
  ElementSpecPtr slotRxS = conf->addElement(ElementPtr(new Slot("slotTx1")));

  conf->hookUp(udpRxS, 0, rxPrintS, 0);
  conf->hookUp(rxPrintS, 0, unmarshalS, 0);
  conf->hookUp(unmarshalS, 0, unBoxS, 0);  
  conf->hookUp(unBoxS, 0, sinkPrintS, 0);
  conf->hookUp(sinkPrintS, 0, demuxS, 0);
  conf->hookUp(demuxS, 0, pong, 0);
  conf->hookUp(demuxS, 1, ping, 1);
  conf->hookUp(demuxS, 2, discardS, 0);
  conf->hookUp(pong, 0, pongPrint, 0);
  conf->hookUp(pongPrint, 0, slotRxS, 0);

  // transmit the PingResponse back to ping requester. Use the 
  // same udp transmitter as before
  conf->hookUp(slotRxS, 0, muxS, 1); 

  // output the ping results (with latency numbers)
  ElementSpecPtr slotRxS1 = conf->addElement(ElementPtr(new Slot("slotRx1")));
  ElementSpecPtr pingResultPrint = conf->addElement(ElementPtr(new Print("PingResultReply")));
  ElementSpecPtr sinkS =
    conf->addElement(ElementPtr(new TimedPullSink("sink", 0)));
  conf->hookUp(ping, 0, pingResultPrint, 0);
  conf->hookUp(pingResultPrint, 0, slotRxS1, 0);
  conf->hookUp(slotRxS1, 0, sinkS, 0);   
 
  PlumberPtr plumber(new Plumber(conf, level));
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
  std::cout << "PingPong test\n";
  LoggerI::Level level = LoggerI::NONE;
  
  if (argc <= 1) {
    std::cout << "Usage: pingpong <number>" << "\n";
    exit(0);
  }

  int mode = atoi(argv[1]);
  if (mode == 1) { // ponger
    testPingPong(atoi(argv[1]), "localhost", level);
  } else { // pinger
    if (argc <= 2) {
      std::cout << "Usage: pingpong <number>" << "\n";
      exit(0);
    } 

    char *d = (char*) malloc(80 * sizeof(char));
    struct hostent *h = gethostbyname(argv[2]);
    sprintf(d, "%s", inet_ntoa(*((struct in_addr *)h->h_addr)));
    string remoteHost(d);
    testPingPong(atoi(argv[1]), remoteHost, level);
    free(d);
  }
  return 0;
}
  

