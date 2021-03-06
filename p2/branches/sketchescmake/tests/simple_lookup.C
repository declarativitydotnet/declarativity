/*
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * Or
 * UC Berkeley EECS Computer Science Division, 387 Soda Hall #1776, 
 * Berkeley, CA,  94707. Attention: P2 Group.
 * 
 *
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */
#include <iostream>
#include <stdlib.h>

#include "tuple.h"
#include "plumber.h"
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
#include "unmarshalField.h"
#include "timedPullSink.h"
#include "slot.h"

void killJoin()
{
  exit(0);
}

struct LookupGenerator : public FunctorSource::Generator
{
  // virtual ~LookupGenerator() {};
  LookupGenerator(string s, string d, string e) : src_(s), dest_(d), event_(e), exit_(false) {};

  TuplePtr operator()() {
    if (exit_) exit(0);
    else exit_ = true;

    TuplePtr tuple = Tuple::mk();
    tuple->append(Val_Str::mk("lookup"));
    tuple->append(Val_Str::mk(src_));

    uint32_t words[ID::WORDS];
    for (uint i = 0; i < ID::WORDS; i++) {
      words[i] = random();
    }
    IDPtr key = ID::mk(words);  

    tuple->append(Val_ID::mk(key));
    tuple->append(Val_Str::mk(dest_));		// WHere the answer is returned
    tuple->append(Val_Str::mk(event_)); 	// the event ID
    tuple->freeze();
    return tuple;
  }

  string src_;
  string dest_;
  string event_;
  mutable bool exit_;
};

void issue_lookup(boost::shared_ptr<LookupGenerator> lookup)
{
  PlumberPtr plumber(new Plumber());
  Plumber::DataflowPtr conf(new Plumber::Dataflow("test"));

  // sending result
  ElementSpecPtr func    = conf->addElement(ElementPtr(new FunctorSource(string("Source"), lookup.get())));
  ElementSpecPtr print   = conf->addElement(ElementPtr(new Print(string("lookup"))));
		       
  ElementSpecPtr encap = conf->addElement(ElementPtr(new PelTransform("encapRequest",
									  "$1 pop \
                                                     $0 ->t $1 append $2 append $3 append $4 append pop"))); // the rest
  ElementSpecPtr marshal = conf->addElement(ElementPtr(new MarshalField("Marshal", 1)));
  ElementSpecPtr route   = conf->addElement(ElementPtr(new StrToSockaddr(string("SimpleLookup"), 0)));
  boost::shared_ptr< Udp > udp(new Udp("Udp", 9999));
  ElementSpecPtr udpTx   = conf->addElement(udp->get_tx());

  conf->hookUp(func, 0, print, 0);
  conf->hookUp(print, 0, encap, 0);
  conf->hookUp(encap, 0, marshal, 0);
  conf->hookUp(marshal, 0, route, 0);
  conf->hookUp(route, 0, udpTx, 0);


  // getting results back
  ElementSpecPtr udpRxS = conf->addElement(udp->get_rx());
  ElementSpecPtr unmarshalS = conf->addElement(ElementPtr(new UnmarshalField(string("Unmarshal:"), 1)));

  // Drop the source address and decapsulate
  ElementSpecPtr unBoxS =
    conf->addElement(ElementPtr(new PelTransform(string("UnBox:"),
                                                    "$1 unboxPop ")));
  ElementSpecPtr recv =
    conf->addElement(ElementPtr(new Print(string("lookupResults:"))));

  ElementSpecPtr slot = conf->addElement(ElementPtr(new Slot("slot")));
  ElementSpecPtr sinkS =
    conf->addElement(ElementPtr(new TimedPullSink("sink", 0)));
  
  conf->hookUp(udpRxS, 0, unmarshalS, 0);
  conf->hookUp(unmarshalS, 0, unBoxS, 0);
  conf->hookUp(unBoxS, 0, recv, 0);
  conf->hookUp(recv, 0, slot, 0);
  conf->hookUp(slot, 0, sinkS, 0);
   
  if (plumber->install(conf) != 0) {
    TELL_ERROR << "** Failed to initialize correct spec\n";
    return;
  }

  // Run the plumber
  eventLoop();

  // Schedule kill
  delayCB(10.0, boost::bind(&killJoin));
}

int main(int argc, char **argv)
{
  Reporting::Level level;
  if (argc < 3) {
    TELL_ERROR << "Usage: simple_lookup logLevel seed event source_ip dest_ip\n";
    exit(0);
  }

  eventLoopInitialize();

  int seed = 0;
  level = Reporting::levelFromName()[string(argv[1])];
  Reporting::setLevel(level);
  seed = atoi(argv[2]);
  srandom(seed);
  issue_lookup(boost::shared_ptr<LookupGenerator>(new LookupGenerator(argv[4], argv[5], argv[3])));

  return 0;
}
  

/*
 * End of file 
 */
