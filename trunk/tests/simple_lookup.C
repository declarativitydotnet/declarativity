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

struct LookupGenerator : public FunctorSource::Generator
{
  // virtual ~LookupGenerator() {};
  LookupGenerator(str s, str d, uint32_t e) : src_(s), dest_(d), event_(e), exit_(false) {};

  TupleRef operator()() const {
    if (exit_) exit(0);
    else exit_ = true;

    TupleRef tuple = Tuple::mk();
    tuple->append(Val_Str::mk("lookup"));
    tuple->append(Val_Str::mk(src_));

    uint zerowords = (uint) ((((double) (ID::WORDS + 1)) * rand()) / RAND_MAX);
    uint32_t words[ID::WORDS];
    for (uint i = 0;
         i < zerowords;
         i++) {
      words[i] = 0;
    }
    for (uint i = zerowords;
         i < ID::WORDS;
         i++) {
      words[i] = rand();
    }
    IDRef key = ID::mk(words)->add(ID::mk((uint) 10));  // Adding 10 to
                                                        // have some
                                                        // lookups locally
                                                        // satisfiable
    tuple->append(Val_ID::mk(key));
    tuple->append(Val_Str::mk(dest_));		// WHere the answer is returned
    tuple->append(Val_UInt32::mk(event_)); 	// the event ID
    tuple->freeze();
    return tuple;
  }

  str src_;
  str dest_;
  uint32_t event_;
  mutable bool exit_;
};

void issue_lookup(LoggerI::Level level, ptr<LookupGenerator> lookup)
{
  Router::ConfigurationRef conf = New refcounted< Router::Configuration >();

  ElementSpecRef func    = conf->addElement(New refcounted< FunctorSource >(str("Source"), lookup));
  ElementSpecRef print   = conf->addElement(New refcounted< Print >(strbuf("lookup")));
		       
  ElementSpecRef encap = conf->addElement(New refcounted< PelTransform >("encapRequest",
									  "$1 pop \
                                                     $0 ->t $1 append $2 append $3 append $4 append pop")); // the rest
  ElementSpecRef marshal = conf->addElement(New refcounted< MarshalField >("Marshal", 1));
  ElementSpecRef route   = conf->addElement(New refcounted< StrToSockaddr >(strbuf("SimpleLookup"), 0));
  ref< Udp >     udp     = New refcounted< Udp >("Udp");
  ElementSpecRef udpTx   = conf->addElement(udp->get_tx());


  conf->hookUp(func, 0, print, 0);
  conf->hookUp(print, 0, encap, 0);
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
  if (argc < 3) {
    std::cout << "Usage: simple_lookup logLevel seed event source_ip dest_ip\n";
    exit(0);
  }

  int seed = 0;
  level = LoggerI::levelFromName[str(argv[1])];
  seed = atoi(argv[2]);
  srand(seed);
  issue_lookup(level, New refcounted<LookupGenerator>(argv[4], argv[5], atoi(argv[3])));

  return 0;
}
  

/*
 * End of file 
 */
