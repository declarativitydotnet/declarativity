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
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <config.h>
#endif /* HAVE_CONFIG_H */
#include <async.h>
#include <arpc.h>
#include <iostream>
#include <math.h>

#include "tuple.h"
#include "router.h"
#include "udp.h"
#include "tupleseq.h"
#include "rcct.h"
#include "rccr.h"
#include "cct.h"
#include "ccr.h"
#include "bw.h"
#include "snetsim.h"
#include "demux.h"
#include "mux.h"
#include "roundRobin.h"
#include "skr.h"
#include "tman.h"
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
#include "loggerI.h"
#include "discard.h"
#include "val_uint32.h"
#include "val_str.h"

#define START_PORT 10000

void hookupSend_RCC(Router::ConfigurationRef conf, str src, bool do_retry,
                    ElementSpecRef dmux_out,  int pdo, ElementSpecRef dmux_in, int pdi,
                    ElementSpecRef rr_out,    int pmo, ElementSpecRef mux_in,  int pmi) {
  // SENDER
  ElementSpecRef dataq  = conf->addElement(New refcounted< Queue >("Data Q", 1000));
  ElementSpecRef seq    = conf->addElement(New refcounted< Sequence >("Sequence", src, pdi));
  ElementSpecRef retry  = conf->addElement(New refcounted< RDelivery >("RETRY", false));
  ElementSpecRef rcct   = conf->addElement(New refcounted< RateCCT >("RateCCT"));
  ElementSpecRef addr   = conf->addElement(New refcounted< PelTransform >("ADDRESS", 
                                                                         "$2 2 field pop swallow pop"));
  ElementSpecRef pstrip = conf->addElement(New refcounted< PelTransform >("PORT STRIP", "$1 unboxPop"));

  conf->hookUp(dmux_out, pdo, pstrip, 0);
  conf->hookUp(pstrip,   0,   dataq,  0);
  if (do_retry) {
    ElementSpecRef rr_retry = conf->addElement(New refcounted< RoundRobin >("RR RETRY", 2));
    ElementSpecRef retryq   = conf->addElement(New refcounted< Queue >("Retry Q", 1000));
    ElementSpecRef strip    = conf->addElement(New refcounted< PelTransform >("STRIP", 
                                                                              "$1 unboxPop"));
    ElementSpecRef retryP   = conf->addElement(New refcounted< Print >("RETRY PRINT"));


    conf->hookUp(dataq,    0, rr_retry, 0);
    conf->hookUp(retry,    1, mux_in,   pmi); 
    conf->hookUp(retry,    2, strip,    0);
    conf->hookUp(strip,    0, retryq,   0);
    conf->hookUp(retryq,   0, retryP,   0);
    conf->hookUp(retryP,   0, rr_retry, 1);
    conf->hookUp(rr_retry, 0, seq,      0);
  }
  else {
    ElementSpecRef rm = conf->addElement(New refcounted< Mux >("RETRY MUX", 2));

    conf->hookUp(dataq, 0, seq,    0);
    conf->hookUp(retry, 1, rm,     0); 
    conf->hookUp(retry, 2, rm,     1); 
    conf->hookUp(rm,    0, mux_in, pmi); 
  }

  conf->hookUp(seq,   0, retry,  0);
  conf->hookUp(retry, 0, rcct,   0);
  conf->hookUp(rcct,  0, addr,   0);
  conf->hookUp(addr,  0, rr_out, pmo);

  conf->hookUp(dmux_in, pdi, rcct,  1);
  conf->hookUp(rcct,    1,   retry, 1);
}

void hookupRecv_RCC(Router::ConfigurationRef conf, ElementSpecRef udprx, 
                ElementSpecRef dmux_in, ElementSpecRef rr_out)
{

  // RECEIVER
  ElementSpecRef unmarshal = conf->addElement(New refcounted< UnmarshalField >("UNMARSHAL FIELD", 1));
  ElementSpecRef unroute   = conf->addElement(New refcounted< PelTransform >("UNROUTE", "$1 unboxPop"));
  ElementSpecRef rccr      = conf->addElement(New refcounted< RateCCR >("RCC Receive"));
  ElementSpecRef respAddr  = conf->addElement(New refcounted< PelTransform >("RESPONSE ADDRESS", 
									     "$0 pop swallow pop"));

  // PACKET RECEIVE DATA FLOW
  conf->hookUp(udprx,     0, unmarshal, 0);
  conf->hookUp(unmarshal, 0, unroute,   0);
  conf->hookUp(unroute,   0, rccr,      0);
  conf->hookUp(rccr,      0, dmux_in,   0);

  // ACK DATA FLOW
  conf->hookUp(rccr, 1, respAddr, 0);
  conf->hookUp(respAddr, 0, rr_out, 1);
}

void hookupSend_CC(Router::ConfigurationRef conf, str src, bool do_retry,
                   ElementSpecRef dmux_out,  int pdo, ElementSpecRef dmux_in, int pdi,
                   ElementSpecRef rr_out,    int pmo, ElementSpecRef mux_in,  int pmi) {
  // SENDER
  ElementSpecRef dataq  = conf->addElement(New refcounted< Queue >("Data Q", 1000));
  ElementSpecRef seq    = conf->addElement(New refcounted< Sequence >("Sequence", src, pdi));
  ElementSpecRef retry  = conf->addElement(New refcounted< RDelivery >("RETRY", do_retry));
  ElementSpecRef cct    = conf->addElement(New refcounted< CCT >("CCT",2,2048));
  ElementSpecRef addr   = conf->addElement(New refcounted< PelTransform >("ADDRESS", 
                                                                         "$1 2 field pop swallow pop"));
  ElementSpecRef pstrip = conf->addElement(New refcounted< PelTransform >("PORT STRIP", "$1 unboxPop"));
  ElementSpecRef rm = conf->addElement(New refcounted< Mux >("RETRY MUX", 2));

  conf->hookUp(dmux_out, pdo, pstrip, 0);	// Strip off the port
  conf->hookUp(pstrip,   0,   dataq,  0);	// Put in data send queue

  conf->hookUp(dataq, 0, seq,    0);		// Data send queue to sequence
  conf->hookUp(seq,   0, retry,  0);		// Plug in the retry
  conf->hookUp(retry, 0, cct,    0);		// Right on down to the cc transmit
  conf->hookUp(cct,   0, addr,   0);		// Extract the next hop address
  conf->hookUp(addr,  0, rr_out, pmo);		// And away she goes

  // INCOMING
  conf->hookUp(dmux_in, pdi, cct,    1);	// Run it through the cc transmit (for acks)
  conf->hookUp(cct,    1,    retry,  1);	// CC will pass along to retry
  conf->hookUp(retry,  1,    rm,     0);	// Regular data channel
  conf->hookUp(retry,  2,    rm,     1); 	// Failure channel
  conf->hookUp(rm,     0,    mux_in, pmi); 	// Merge up to the router
}

void hookupRecv_CC(Router::ConfigurationRef conf, ElementSpecRef udprx, 
                ElementSpecRef dmux_in, ElementSpecRef rr_out)
{

  // RECEIVER
  ElementSpecRef unmarshal = conf->addElement(New refcounted< UnmarshalField >("UNMARSHAL FIELD", 1));
  ElementSpecRef unroute   = conf->addElement(New refcounted< PelTransform >("UNROUTE", "$1 unboxPop"));
  ElementSpecRef ccr       = conf->addElement(New refcounted< CCR >("CC Receive"));
  ElementSpecRef respAddr  = conf->addElement(New refcounted< PelTransform >("RESPONSE ADDRESS", 
									     "$0 pop swallow pop"));

  // PACKET RECEIVE DATA FLOW
  conf->hookUp(udprx,     0, unmarshal, 0);
  conf->hookUp(unmarshal, 0, unroute,   0);
  conf->hookUp(unroute,   0, ccr,      0);
  conf->hookUp(ccr,       0, dmux_in,   0);

  // ACK DATA FLOW
  conf->hookUp(ccr, 1, respAddr, 0);
  conf->hookUp(respAddr, 0, rr_out, 1);
}

void runNode(int nodeid, int ltime, int nodes, double drop, bool emulab)
{
  Router::ConfigurationRef conf = New refcounted< Router::Configuration >();
  str my_addr = strbuf() << "localhost:" << (START_PORT + nodeid);
  if (emulab) {
    char buf[64];
    sprintf(buf, "node%04d:%d\n", nodeid, (START_PORT+nodeid));
    my_addr = str(buf);
  }

  // Create the router routing table, specifically for Emulab.
  ptr < SimpleKeyRouter > skrp = New refcounted< SimpleKeyRouter >("SimpleKeyRoute", 
                                                                   Val_UInt32::mk(nodeid));
  for (uint i = 0; pow(2, i) < nodes; i++) {
    uint nid = (nodeid + uint(pow(2, i))) % nodes;
    char buf[64];
    if (emulab) sprintf(buf, "node%04d:%d\n", nid, (START_PORT+nid));
    else        sprintf(buf, "localhost:%d\n", (START_PORT + nid));
    skrp->route(Val_UInt32::mk(nid), Val_Str::mk(buf));
  }
  ptr < vec<ValueRef> > ports = skrp->routes();

  ptr<Udp>       udp   = New refcounted<Udp>("UDP", (START_PORT+nodeid));
  ElementSpecRef udpTx = conf->addElement(udp->get_tx());
  ElementSpecRef udpRx = conf->addElement(udp->get_rx());
  ElementSpecRef respQ = conf->addElement(New refcounted< Queue >("Response Q", 1000));

  ElementSpecRef skr      = conf->addElement(skrp);
  ElementSpecRef tman     = conf->addElement(New refcounted<TrafficManager>("TGEN", my_addr, nodeid, nodes, ltime));
  ElementSpecRef rr_out   = conf->addElement(New refcounted< RoundRobin >("UDP RR", ports->size()+2));
  ElementSpecRef mux_in   = conf->addElement(New refcounted< Mux >("MUX IN", ports->size()+1));

  ElementSpecRef portA    = conf->addElement(New refcounted< PelTransform >("SKR PORT ADJUST", 
									     "$0 1 field pop swallow pop"));
  ElementSpecRef dmux_out  = conf->addElement(New refcounted< Demux >("CC OUT DEMUX", ports));
  ElementSpecRef dmux_in   = conf->addElement(New refcounted< Demux >("CC IN DEMUX", ports, 1));
  ElementSpecRef dout_errP = conf->addElement(New refcounted< Print >("DEMUX OUT ERROR PORT MATCH"));
  ElementSpecRef dout_dis  = conf->addElement(New refcounted< Discard >("DISCARD ERROR CASE: DOUT"));
  conf->hookUp(dmux_out, ports->size(), dout_errP, 0);
  conf->hookUp(dout_errP, 0, dout_dis, 0);

  conf->hookUp(dmux_in, ports->size(), mux_in, ports->size());

  // SENDER PACKAGE UP AND SEND TUPLE
  ElementSpecRef sendP    = conf->addElement(New refcounted< Print >("SEND PRINT"));
  ElementSpecRef recvP    = conf->addElement(New refcounted< Print >("RECEIVE PRINT"));
  ElementSpecRef marshal  = conf->addElement(New refcounted< MarshalField >("MARHSAL", 1));
  ElementSpecRef route    = conf->addElement(New refcounted< StrToSockaddr >("sock2addr", 0));
  ElementSpecRef netsim   = conf->addElement(New refcounted< SimpleNetSim >("Net Sim (Sender)", 
									    0, 0, drop));
  ElementSpecRef respAddr = conf->addElement(New refcounted< PelTransform >("LOOKUP RESPONSE ADDRESS", 
									     "$1 pop swallow pop"));
  // SEND CHANNEL
  conf->hookUp(rr_out,  0, marshal, 0);
  conf->hookUp(marshal, 0, route,   0);
  conf->hookUp(route,   0, netsim,  0);
  conf->hookUp(netsim,  0, udpTx,   0);

  // LOOKUP GENERATION
  conf->hookUp(tman,     0, skr,      0); // Traffic Manager to Router channel
  conf->hookUp(tman,     1, respQ,    0); // Traffic Manager lookup response channel
  conf->hookUp(respQ,    0, respAddr, 0); 
  conf->hookUp(respAddr, 0, rr_out,   0); 
  conf->hookUp(skr,      1, tman,     0); // Router delivery of lookup destined for this host
  conf->hookUp(skr,      0, portA,    0); // Router out channel print
  conf->hookUp(portA,    0, sendP,    0); // Router out channel print
  conf->hookUp(sendP,    0, dmux_out, 0); // Router out channel to CC demux
  conf->hookUp(mux_in,   0, recvP,    0); // Incoming lookups, responses, and failed attempts
  conf->hookUp(recvP,    0, skr,      1);

  // HOOKUP EVERYTHING IN THE MIDDLE
  for (uint i = 0; i < ports->size(); i++) {
    hookupSend_CC(conf, my_addr, false, dmux_out, i, dmux_in, i, rr_out, i+2, mux_in, i);
  }
  hookupRecv_CC(conf, udpRx, dmux_in, rr_out);


  // Create router and run.
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
  bool forkNodes = false;
  bool emulab    = false;
  double drop    = 0.;

  argv++; argc--;
  if (argc < 3) {
    std::cout << "Usage: perfectChord [-f] [-e] [-d drop_prob] <id> <lookup_time> <node_count>\n";
    exit(0);
  }

  if (argv[0][0] == '-' && argv[0][1] == 'f') {
    forkNodes = true;
    argv++; argc--;
  }

  if (argv[0][0] == '-' && argv[0][1] == 'e') {
    emulab = true;
    argv++; argc--;
  }

  if (argv[0][0] == '-' && argv[0][1] == 'd') {
    if (strlen(argv[0]) > 2) drop = atof(argv[0]+2);
    else {
      drop = atof(argv[1]);
      argv++; argc--;
    }
    argv++; argc--;
  }
    
  if (argc != 3) {
    std::cout << "Usage: perfectChord [-e] [-d drop_prob] <id> <lookup_time> <node_count>\n";
    exit(0);
  }
  int   id    = atoi(argv[0]);
  int   ltime = atoi(argv[1]);
  int   nodes = atoi(argv[2]);

  if (forkNodes) {
    int   status = 0;
    for (int i = 0; i < nodes; i++) {
      if (fork() == 0)
        runNode(id+i, ltime, nodes, drop, emulab);
    }
    wait(&status);
    std::cerr << "PERFECT CHORD EXIT WITH STATUS: " << status << std::endl;
  } else runNode(id, ltime, nodes, drop, emulab);

  return 0;
}
  

/*
 * End of file 
 */
