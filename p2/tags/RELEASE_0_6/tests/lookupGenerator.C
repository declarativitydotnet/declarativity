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
#include "timedPullPush.h"

struct LookupGenerator : public FunctorSource::Generator
{
  LookupGenerator(string host,
                  int firstPort,
                  int ports,
                  string eventPrefix)
    : _h(host),
      _f(firstPort),
      _p(ports),
      _current(0),
      _e(0),
      _prefix(eventPrefix)
  {};
  
  TuplePtr operator()() {
    TuplePtr tuple = Tuple::mk();
    tuple->append(Val_Str::mk("lookup"));

    // The source
    ostringstream node;
    node << string(_h) << ":" << (_f + _current);
    tuple->append(Val_Str::mk(node.str()));

    uint32_t words[ID::WORDS];
    for (uint i = 0;
         i < ID::WORDS;
         i++) {
      words[i] = random();
    }
    ostringstream oss;
    oss << _prefix << ":" << _e;
    IDPtr key = ID::mk(words);  
    tuple->append(Val_ID::mk(key));

    tuple->append(Val_Str::mk(node.str()));		// WHere the answer is returned
    tuple->append(Val_Str::mk(oss.str())); // the event ID
    tuple->freeze();
    _e++;
    _current = (_current + 1) % _p;
    return tuple;
  }

  string _h;
  int _f;
  int _p;
  int _current;
  uint64_t _e;
  string _prefix;
};

void issue_lookup(LoggerI::Level level, boost::shared_ptr<LookupGenerator> lookup,
                  double delay, int times)
{
  eventLoopInitialize();
  PlumberPtr plumber(new Plumber(level));
  Plumber::DataflowPtr conf = plumber->new_dataflow("test");

  ElementSpecPtr func    = conf->addElement(ElementPtr(new FunctorSource(string("Source"), lookup.get())));
  ElementSpecPtr print   = conf->addElement(ElementPtr(new Print(string("lookup"))));
  ElementSpecPtr pushS =
    conf->addElement(ElementPtr(new TimedPullPush(string("Push:"),
                                                     delay, // run then
                                                     times // run once
                                                     )));

  // And a slot from which to pull
  ElementSpecPtr slotS =
    conf->addElement(ElementPtr(new Slot(string("JoinEventSlot:"))));
		       
  ElementSpecPtr encap = conf->addElement(ElementPtr(new PelTransform("encapRequest",
									  "$1 pop \
                                                     $0 ->t $1 append $2 append $3 append $4 append pop"))); // the rest
  ElementSpecPtr marshal = conf->addElement(ElementPtr(new MarshalField("Marshal", 1)));
  ElementSpecPtr route   = conf->addElement(ElementPtr(new StrToSockaddr(string("SimpleLookup"), 0)));
  boost::shared_ptr< Udp > udp(new Udp("Udp"));
  ElementSpecPtr udpTx   = conf->addElement(udp->get_tx());


  conf->hookUp(func, 0, print, 0);
  conf->hookUp(print, 0, pushS, 0);
  conf->hookUp(pushS, 0, slotS, 0);
  conf->hookUp(slotS, 0, encap, 0);
  conf->hookUp(encap, 0, marshal, 0);
  conf->hookUp(marshal, 0, route, 0);
  conf->hookUp(route, 0, udpTx, 0);
   
  if (plumber->install(conf) != 0) {
    std::cout << "** Failed to initialize correct spec\n";
    return;
  }

  // Run the plumber
  eventLoop();
}

int main(int argc, char **argv)
{
  LoggerI::Level level = LoggerI::ALL;
  if (argc < 9) {
    std::cout << "Usage: lookupGenerator logLevel seed host firstPort ports delay times eventPrefix \n";
    exit(0);
  }

  int seed = 0;
  level = LoggerI::levelFromName[string(argv[1])];
  seed = atoi(argv[2]);
  srandom(seed);
  issue_lookup(level, boost::shared_ptr<LookupGenerator>(new LookupGenerator(argv[3],
                                                                             atoi(argv[4]),
                                                                             atoi(argv[5]),
                                                                             argv[8])),
                                                                             atof(argv[6]),
                                                                             atoi(argv[7]));

  return 0;
}
  

/*
 * End of file 
 */
