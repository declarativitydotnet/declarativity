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
 * DESCRIPTION: Send simple message via UDP
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
#include "tupleSource.h"
#include "timedPullSink.h"
#include "ping.h"
#include "pong.h"
#include "slot.h"
#include "val_str.h"
#include "val_tuple.h"
#include "val_uint64.h"
#include "demux.h"
#include "roundRobin.h"
#include "marshalField.h"
#include "unmarshalField.h"
#include "store.h"
#include "timedPullPush.h"
#include "discard.h"

void
listenForMessages(std::string ipAddress,
                  int port)
{
  eventLoopInitialize();

  // The sending data flow
  Plumber::ConfigurationPtr conf(new Plumber::Configuration());    

  Udp udpOut("ListenerUDP", port); // port of the sender    
  std::vector<TuplePtr> buffer;


  ElementSpecPtr udpRxS =
    conf->addElement(udpOut.get_rx());
  ElementSpecPtr udpRxPrintS =
    conf->addElement(ElementPtr(new Print("After Udp::Rx")));
  ElementSpecPtr unmarshalS =
    conf->addElement(ElementPtr(new UnmarshalField("unmarshal", 1)));
  ElementSpecPtr sinkPrintS =
    conf->addElement(ElementPtr(new Print("After Unmarshal")));
  ElementSpecPtr discardS =
    conf->addElement(ElementPtr(new Discard("defaultSink")));

  // Link everything
  conf->hookUp(udpRxS, 0, udpRxPrintS, 0);
  conf->hookUp(udpRxPrintS, 0, unmarshalS, 0);
  conf->hookUp(unmarshalS, 0, sinkPrintS, 0);
  conf->hookUp(sinkPrintS, 0, discardS, 0);

  // Put the plumber together
  PlumberPtr plumber(new Plumber(conf, LoggerI::ALL));
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
  std::cout << "Simple message listener on UDP\n";
  
  if (argc != 3) {
    std::cerr << "Usage: udpListen <myIP> <myPort>\n";
    exit(0);
  }


  std::string myAddress(argv[1]);
  int port = atoi(argv[2]);

  std::cout << "Listening for messages to address "
            << myAddress 
            << " and port "
            << port
            << "\n";

  listenForMessages(myAddress, port);
  return 0;
}
  

