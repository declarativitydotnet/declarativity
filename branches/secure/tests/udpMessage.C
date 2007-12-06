// -*- c-basic-offset: 2; related-file-name: "" -*-
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
#include "pelTransform.h"
#include "unmarshal.h"
#include "staticTupleSource.h"
#include "slot.h"
#include "val_str.h"
#include "val_tuple.h"
#include "val_int64.h"
#include "demux.h"
#include "roundRobin.h"
#include "marshalField.h"
#include "unmarshalField.h"
#include "timedPullPush.h"
#include "discard.h"

void
sendMessages(std::string udpAddress)
{
  eventLoopInitialize();

  // The sending data flow
  PlumberPtr plumber(new Plumber());
  Plumber::DataflowPtr conf(new Plumber::Dataflow("test"));

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
    conf->addElement(ElementPtr(new StaticTupleSource("Source",
                                                      tuple)));
  ElementSpecPtr pusherS =
    conf->addElement(ElementPtr(new TimedPullPush("Push", 0.5, 0)));
  ElementSpecPtr slotS =
    conf->addElement(ElementPtr(new Slot("Slot")));
  ElementSpecPtr marshalS =
    conf->addElement(ElementPtr(new MarshalField("Marshal", 1)));
  ElementSpecPtr routeS =
    conf->addElement(ElementPtr(new StrToSockaddr("Router", 0)));
  ElementSpecPtr udpTxPrintS =
    conf->addElement(ElementPtr(new Print("Before Udp::Tx")));
  ElementSpecPtr udpTxS = conf->addElement(udpOut.get_tx());

  // Link everything
  conf->hookUp(sourceS, 0, pusherS, 0);
  conf->hookUp(pusherS, 0, slotS, 0);
  conf->hookUp(slotS, 0, marshalS, 0);
  conf->hookUp(marshalS, 0, routeS, 0);
  conf->hookUp(routeS, 0, udpTxPrintS, 0);
  conf->hookUp(udpTxPrintS, 0, udpTxS, 0);
  

  // Put the plumber together
  if (plumber->install(conf) == 0) {
    TELL_INFO << "Correctly initialized.\n";
  } else {
    TELL_ERROR << "** Failed to initialize correct spec\n";
  }

  // Run the plumber
  eventLoop();
}


int main(int argc, char **argv)
{
  TELL_INFO << "Simple message via UDP\n";
  
  if (argc != 2) {
    TELL_ERROR << "Usage: udpMessage <destIP:destPort>\n";
    exit(0);
  }


  std::string myAddress(argv[1]);

  TELL_INFO << "Sending messages to address "
            << myAddress 
            << "\n";

  sendMessages(myAddress);
  return 0;
}
  

