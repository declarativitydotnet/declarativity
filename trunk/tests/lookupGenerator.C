/*
 * Copyright (c) 2004 Intel Corporation. All rights reserved.
 *
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 *
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */
#include <async.h>
#include <arpc.h>
#include <iostream>
#include <stdlib.h>

#include "tuple.h"
#include "router.h"
#include "val_uint32.h"
#include "val_str.h"
#include "val_id.h"

#include "functorSource.h"
#include "print.h"
#include "marshal.h"
#include "udp.h"
#include "strToSockaddr.h"
#include "slot.h"
#include "loggerI.h"
#include "pelTransform.h"
#include "marshalField.h"
#include "timedPullPush.h"

struct LookupGenerator : public FunctorSource::Generator
{
  LookupGenerator(str host,
                  int firstPort,
                  int ports)
    : _h(host),
      _f(firstPort),
      _p(ports),
      _current(0),
      _e(0)
  {};
  
  TupleRef operator()() {
    TupleRef tuple = Tuple::mk();
    tuple->append(Val_Str::mk("lookup"));

    // The source
    str node = strbuf(_h) << ":" << (_f + _current);
    tuple->append(Val_Str::mk(node));

    uint32_t words[ID::WORDS];
    for (uint i = 0;
         i < ID::WORDS;
         i++) {
      words[i] = random();
    }
    IDRef key = ID::mk(words);  
    tuple->append(Val_ID::mk(key));

    tuple->append(Val_Str::mk(node));		// WHere the answer is returned
    tuple->append(Val_Str::mk(strbuf() << _e)); 	// the event ID
    tuple->freeze();
    _e++;
    _current = (_current + 1) % _p;
    return tuple;
  }

  str _h;
  int _f;
  int _p;
  int _current;
  uint64_t _e;
};

void issue_lookup(LoggerI::Level level, ptr<LookupGenerator> lookup,
                  double delay, int times)
{
  Router::ConfigurationRef conf = New refcounted< Router::Configuration >();

  ElementSpecRef func    = conf->addElement(New refcounted< FunctorSource >(str("Source"), lookup));
  ElementSpecRef print   = conf->addElement(New refcounted< Print >(strbuf("lookup")));
  ElementSpecRef pushS =
    conf->addElement(New refcounted< TimedPullPush >(strbuf("Push:"),
                                                     delay, // run then
                                                     times // run once
                                                     ));

  // And a slot from which to pull
  ElementSpecRef slotS =
    conf->addElement(New refcounted< Slot >(strbuf("JoinEventSlot:")));
		       
  ElementSpecRef encap = conf->addElement(New refcounted< PelTransform >("encapRequest",
									  "$1 pop \
                                                     $0 ->t $1 append $2 append $3 append $4 append pop")); // the rest
  ElementSpecRef marshal = conf->addElement(New refcounted< MarshalField >("Marshal", 1));
  ElementSpecRef route   = conf->addElement(New refcounted< StrToSockaddr >(strbuf("SimpleLookup"), 0));
  ref< Udp >     udp     = New refcounted< Udp >("Udp");
  ElementSpecRef udpTx   = conf->addElement(udp->get_tx());


  conf->hookUp(func, 0, print, 0);
  conf->hookUp(print, 0, pushS, 0);
  conf->hookUp(pushS, 0, slotS, 0);
  conf->hookUp(slotS, 0, encap, 0);
  conf->hookUp(encap, 0, marshal, 0);
  conf->hookUp(marshal, 0, route, 0);
  conf->hookUp(route, 0, udpTx, 0);
   
  RouterRef router = New refcounted< Router >(conf, level);
  if (router->initialize(router) != 0) {
    std::cout << "** Failed to initialize correct spec\n";
    return;
  }

  // Activate the router
  router->activate();

  // Run the router
  amain();
}

int main(int argc, char **argv)
{
  LoggerI::Level level = LoggerI::ALL;
  if (argc < 8) {
    std::cout << "Usage: lookupGenerator logLevel seed host firstPort ports delay times \n";
    exit(0);
  }

  int seed = 0;
  level = LoggerI::levelFromName[str(argv[1])];
  seed = atoi(argv[2]);
  srandom(seed);
  issue_lookup(level, New refcounted<LookupGenerator>(argv[3],
                                                      atoi(argv[4]),
                                                      atoi(argv[5])),
               atof(argv[6]),
               atoi(argv[7]));

  return 0;
}
  

/*
 * End of file 
 */
