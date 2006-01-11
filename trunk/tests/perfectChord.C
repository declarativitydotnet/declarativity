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

void hookupSend_RCC(Router::ConfigurationPtr conf, string src, bool do_retry,
                    ElementSpecPtr dmux_out,  int pdo, ElementSpecPtr dmux_in, int pdi,
                    ElementSpecPtr rr_out,    int pmo, ElementSpecPtr mux_in,  int pmi) {
  // SENDER
  ElementSpecPtr dataq  = conf->addElement(ElementPtr(new Queue("Data Q", 1000)));
  ElementSpecPtr seq    = conf->addElement(ElementPtr(new Sequence("Sequence", src, pdi)));
  ElementSpecPtr retry  = conf->addElement(ElementPtr(new RDelivery("RETRY", false)));
  ElementSpecPtr rcct   = conf->addElement(ElementPtr(new RateCCT("RateCCT")));
  ElementSpecPtr addr   = conf->addElement(ElementPtr(new PelTransform("ADDRESS", 
                                                                         "$2 2 field pop swallow pop")));
  ElementSpecPtr pstrip = conf->addElement(ElementPtr(new PelTransform("PORT STRIP", "$1 unboxPop")));
  ElementSpecPtr rcount = conf->addElement(ElementPtr(new PelTransform("RETRY ADD", 
                                                                          "$0 pop $1 pop $2 pop \
                                                                           $3 pop $4 pop $5 pop \
                                                                           $6 pop $7 1 + pop")));

  conf->hookUp(dmux_out, pdo, pstrip, 0);
  conf->hookUp(pstrip,   0,   dataq,  0);
  if (do_retry) {
    ElementSpecPtr rr_retry = conf->addElement(ElementPtr(new RoundRobin("RR RETRY", 2)));
    ElementSpecPtr retryq   = conf->addElement(ElementPtr(new Queue("Retry Q", 1000)));
    ElementSpecPtr strip    = conf->addElement(ElementPtr(new PelTransform("STRIP", 
                                                                              "$1 unboxPop")));


    conf->hookUp(dataq,    0, rr_retry, 0);
    conf->hookUp(retry,    1, mux_in,   pmi); 
    conf->hookUp(retry,    2, strip,    0);
    conf->hookUp(strip,    0, retryq,   0);
    conf->hookUp(retryq,   0, rr_retry, 1);
    conf->hookUp(rr_retry, 0, seq,      0);
  }
  else {
    ElementSpecPtr rm = conf->addElement(ElementPtr(new Mux("RETRY MUX", 2)));

    conf->hookUp(dataq, 0, seq,    0);
    conf->hookUp(retry, 1, rm,     0); 
    conf->hookUp(retry, 2, rm,     1); 
    conf->hookUp(rm,    0, mux_in, pmi); 
  }
  ElementSpecPtr retryP   = conf->addElement(ElementPtr(new Print("RETRY INC PRINT")));

  conf->hookUp(seq,   0, retryP,  0);		// Plug in the retry
  // conf->hookUp(retryP, 0, retry,  0);		// Plug in the retry
  conf->hookUp(retryP, 0, rcount, 0);		// Plug in the retry
  conf->hookUp(rcount, 0, retry,  0);		// Plug in the retry
  conf->hookUp(retry,  0, rcct,   0);
  conf->hookUp(rcct,   0, addr,   0);
  conf->hookUp(addr,   0, rr_out, pmo);

  conf->hookUp(dmux_in, pdi, rcct,  1);
  conf->hookUp(rcct,    1,   retry, 1);
}

void hookupRecv_RCC(Router::ConfigurationPtr conf, ElementSpecPtr udprx, 
                ElementSpecPtr dmux_in, ElementSpecPtr rr_out)
{

  // RECEIVER
  ElementSpecPtr unmarshal = conf->addElement(ElementPtr(new UnmarshalField("UNMARSHAL FIELD", 1)));
  ElementSpecPtr unroute   = conf->addElement(ElementPtr(new PelTransform("UNROUTE", "$1 unboxPop")));
  ElementSpecPtr rccr      = conf->addElement(ElementPtr(new RateCCR("RCC Receive")));
  ElementSpecPtr respAddr  = conf->addElement(ElementPtr(new PelTransform("RESPONSE ADDRESS", 
									     "$0 pop swallow pop")));

  // PACKET RECEIVE DATA FLOW
  conf->hookUp(udprx,     0, unmarshal, 0);
  conf->hookUp(unmarshal, 0, unroute,   0);
  conf->hookUp(unroute,   0, rccr,      0);
  conf->hookUp(rccr,      0, dmux_in,   0);

  // ACK DATA FLOW
  conf->hookUp(rccr, 1, respAddr, 0);
  conf->hookUp(respAddr, 0, rr_out, 1);
}

void hookupSend_CC(Router::ConfigurationPtr conf, string src, bool do_retry,
                   ElementSpecPtr dmux_out,  int pdo, ElementSpecPtr dmux_in, int pdi,
                   ElementSpecPtr rr_out,    int pmo, ElementSpecPtr mux_in,  int pmi) {
  // SENDER
  ElementSpecPtr dataq  = conf->addElement(ElementPtr(new Queue("Data Q", 1000)));
  ElementSpecPtr seq    = conf->addElement(ElementPtr(new Sequence("Sequence", src, pdi)));
  ElementSpecPtr retry  = conf->addElement(ElementPtr(new RDelivery("RETRY", do_retry)));
  ElementSpecPtr rcount  = conf->addElement(ElementPtr(new PelTransform("RETRY ADD", 
                                                                          "$0 pop $1 pop $2 pop \
                                                                           $3 pop $4 pop $5 pop \
                                                                           $6 pop $7 1 + pop")));
  ElementSpecPtr cct    = conf->addElement(ElementPtr(new CCT("CCT",2,2048)));
  ElementSpecPtr addr   = conf->addElement(ElementPtr(new PelTransform("ADDRESS", 
                                                                         "$1 2 field pop swallow pop")));
  ElementSpecPtr pstrip = conf->addElement(ElementPtr(new PelTransform("PORT STRIP", "$1 unboxPop")));
  ElementSpecPtr rm = conf->addElement(ElementPtr(new Mux("RETRY MUX", 2)));

  conf->hookUp(dmux_out, pdo, pstrip, 0);	// Strip off the port
  conf->hookUp(pstrip,   0,   dataq,  0);	// Put in data send queue

  conf->hookUp(dataq, 0, seq,    0);		// Data send queue to sequence
  if (!do_retry) {
    conf->hookUp(seq,   0, rcount,  0);		// Plug in the retry
    conf->hookUp(rcount,0, retry,  0);		// Plug in the retry
    conf->hookUp(retry, 0, cct, 0);		// Right on down to the cc transmit
  }
  else {
    conf->hookUp(seq,    0, retry,  0);		// Plug in the retry
    conf->hookUp(retry,  0, rcount, 0);		// Plug in the retry
    conf->hookUp(rcount, 0, cct,    0);		// Right on down to the cc transmit
  }
  conf->hookUp(cct,   0, addr,   0);		// Extract the next hop address
  conf->hookUp(addr,  0, rr_out, pmo);		// And away she goes

  // INCOMING
  conf->hookUp(dmux_in, pdi, cct,    1);	// Run it through the cc transmit (for acks)
  conf->hookUp(cct,    1,    retry,  1);	// CC will pass along to retry
  conf->hookUp(retry,  1,    rm,     0);	// Regular data channel
  conf->hookUp(retry,  2,    rm,     1); 	// Failure channel
  conf->hookUp(rm,     0,    mux_in, pmi); 	// Merge up to the router
}

void hookupRecv_CC(Router::ConfigurationPtr conf, ElementSpecPtr udprx, 
                   ElementSpecPtr dmux_in, ElementSpecPtr rr_out)
{

  // RECEIVER
  ElementSpecPtr unmarshal = conf->addElement(ElementPtr(new UnmarshalField("UNMARSHAL FIELD", 1)));
  ElementSpecPtr unroute   = conf->addElement(ElementPtr(new PelTransform("UNROUTE", "$1 unboxPop")));
  ElementSpecPtr ccr       = conf->addElement(ElementPtr(new CCR("CC Receive")));
  ElementSpecPtr respAddr  = conf->addElement(ElementPtr(new PelTransform("RESPONSE ADDRESS", 
									     "$0 pop swallow pop")));

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
  eventLoopInitialize();
  Router::ConfigurationPtr conf(new Router::Configuration());
  string my_addr;
  ostringstream oss;
  oss << "localhost:" << (START_PORT + nodeid);
  my_addr = oss.str();
  if (emulab) {
    char buf[64];
    sprintf(buf, "node%04d:%d\n", nodeid, (START_PORT+nodeid));
    my_addr = string(buf);
  }
  // std::cerr << "MY ADDRESS: " << my_addr << std::endl;

  // Create the router routing table, specifically for Emulab.
  boost::shared_ptr < SimpleKeyRouter > skrp(new SimpleKeyRouter("SimpleKeyRoute", Val_UInt32::mk(nodeid)));
  for (uint i = 0; pow(2, i) < nodes; i++) {
    uint nid = (nodeid + uint(pow(2, i))) % nodes;
    char buf[64];
    if (emulab) sprintf(buf, "node%04d:%d\n", nid, (START_PORT+nid));
    else        sprintf(buf, "localhost:%d\n", (START_PORT + nid));
    skrp->route(Val_UInt32::mk(nid), Val_Str::mk(buf));
    // std::cerr << "FINGER( " << i << " ): " << string(buf) << std::endl;
  }
  boost::shared_ptr < std::vector<ValuePtr> > ports(skrp->routes());

  boost::shared_ptr<Udp> udp(new Udp("UDP", (START_PORT+nodeid)));
  ElementSpecPtr udpTx = conf->addElement(udp->get_tx());
  ElementSpecPtr udpRx = conf->addElement(udp->get_rx());
  ElementSpecPtr respQ = conf->addElement(ElementPtr(new Queue("Response Q", 1000)));

  ElementSpecPtr skr      = conf->addElement(skrp);
  ElementSpecPtr tman     = conf->addElement(ElementPtr(new TrafficManager("TGEN", my_addr, nodeid, nodes, ltime)));
  ElementSpecPtr rr_out   = conf->addElement(ElementPtr(new  RoundRobin("UDP RR", ports->size()+2)));
  ElementSpecPtr mux_in   = conf->addElement(ElementPtr(new  Mux("MUX IN", ports->size()+1)));

  ElementSpecPtr portA    = conf->addElement(ElementPtr(new  PelTransform("SKR PORT ADJUST", 
									     "$0 1 field pop swallow pop")));
  ElementSpecPtr dmux_out  = conf->addElement(ElementPtr(new  Demux("CC OUT DEMUX", ports)));
  ElementSpecPtr dmux_in   = conf->addElement(ElementPtr(new  Demux("CC IN DEMUX", ports, 1)));
  ElementSpecPtr dout_errP = conf->addElement(ElementPtr(new  Print("DEMUX OUT ERROR PORT MATCH")));
  ElementSpecPtr dout_dis  = conf->addElement(ElementPtr(new  Discard("DISCARD ERROR CASE: DOUT")));
  conf->hookUp(dmux_out, ports->size(), dout_errP, 0);
  conf->hookUp(dout_errP, 0, dout_dis, 0);

  ElementSpecPtr hopcntI   = conf->addElement(ElementPtr(new  PelTransform("HOP COUNT INCREMENT", 
                                                                          "$0 pop $1 pop $2 pop \
                                                                           $3 pop $4 pop $5 pop \
                                                                           $6 1 + pop $7 pop")));
  ElementSpecPtr hopcntP   = conf->addElement(ElementPtr(new  Print("HOPCOUNT INC PRINT")));

  // conf->hookUp(dmux_in, ports->size(), mux_in, ports->size());
  conf->hookUp(dmux_in, ports->size(), hopcntP, 0);
  conf->hookUp(hopcntP, 0, hopcntI, 0);
  conf->hookUp(hopcntI, 0, mux_in, ports->size());

  // SENDER PACKAGE UP AND SEND TUPLE
  ElementSpecPtr sendP    = conf->addElement(ElementPtr(new  Print("SEND PRINT")));
  ElementSpecPtr marshal  = conf->addElement(ElementPtr(new  MarshalField("MARHSAL", 1)));
  ElementSpecPtr route    = conf->addElement(ElementPtr(new  StrToSockaddr("sock2addr", 0)));
  ElementSpecPtr netsim   = conf->addElement(ElementPtr(new  SimpleNetSim("Net Sim (Sender)", 
									    0, 0, drop)));
  ElementSpecPtr respAddr = conf->addElement(ElementPtr(new  PelTransform("LOOKUP RESPONSE ADDRESS", 
									     "$1 pop swallow pop")));
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
  conf->hookUp(mux_in,   0, skr,     1); // Incoming lookups, responses, and failed attempts

  // HOOKUP EVERYTHING IN THE MIDDLE
  for (uint i = 0; i < ports->size(); i++) {
    hookupSend_RCC(conf, my_addr, true, dmux_out, i, dmux_in, i, rr_out, i+2, mux_in, i);
  }
  hookupRecv_RCC(conf, udpRx, dmux_in, rr_out);


  // Create router and run.
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
