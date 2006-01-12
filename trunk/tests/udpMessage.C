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
#include "router.h"
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
sendMessages(std::string udpAddress)
{
  eventLoopInitialize();

  // The sending data flow
  Router::ConfigurationPtr conf(new Router::Configuration());    

  Udp udpOut("9999", 9999); // port of the sender    
  std::vector<TuplePtr> buffer;

  // The message contents
  TuplePtr payload = Tuple::mk();
  payload->append(Val_Str::mk("Payload"));
  payload->freeze();  

  // The input tuple
  TuplePtr tuple = Tuple::mk();
  tuple->append(Val_Str::mk(udpAddress)); // target
  tuple->append(Val_Tuple::mk(payload)); // payload
  tuple->freeze();  

  ElementSpecPtr sourceS =
    conf->addElement(ElementPtr(new TupleSource("Source", tuple)));
  ElementSpecPtr pusherS =
    conf->addElement(ElementPtr(new TimedPullPush("Push", 0.5)));
  ElementSpecPtr slotS =
    conf->addElement(ElementPtr(new Slot("Slot")));
  ElementSpecPtr marshalS =
    conf->addElement(ElementPtr(new MarshalField("Marshal", 1)));
  ElementSpecPtr routeS =
    conf->addElement(ElementPtr(new StrToSockaddr("Router", 0)));
  ElementSpecPtr udpTxS = conf->addElement(udpOut.get_tx());

  // Link everything
  conf->hookUp(sourceS, 0, pusherS, 0);
  conf->hookUp(pusherS, 0, slotS, 0);
  conf->hookUp(slotS, 0, marshalS, 0);
  conf->hookUp(marshalS, 0, routeS, 0);
  conf->hookUp(routeS, 0, udpTxS, 0);
  

  // Put the router together
  RouterPtr router(new Router(conf, LoggerI::ALL));
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
  std::cout << "Simple message via UDP\n";
  
  if (argc != 2) {
    std::cerr << "Usage: udpMessage <destIP:destPort>\n";
    exit(0);
  }


  std::string myAddress(argv[1]);

  std::cout << "Sending messages to address "
            << myAddress 
            << "\n";

  sendMessages(myAddress);
  return 0;
}
  

