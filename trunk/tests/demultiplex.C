/*
 * @(#)$Id$
 *
 *
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * Or
 * UC Berkeley EECS Computer Science Division, 387 Soda Hall #1776, 
 * Berkeley, CA,  94707. Attention: P2 Group.
 * 
 * DESCRIPTION: Tests for static and live demultiplexing
 *
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */
#include <iostream>

#include "tuple.h"
#include "plumber.h"
#include "val_int32.h"

#include "print.h"
#include "timedPushSource.h"
#include "discard.h"
#include "pelTransform.h"
#include "demux.h"

/** Static demux based on first element.  Two dataflows are registered,
    one per demux value (a string).  Then a source dataflow produces
    random sequences of packets switching between demux values. */
void testStaticDemux()
{
  std::cout << "\nCHECK STATIC DEMUX\n";

  Plumber::ConfigurationPtr conf(new Plumber::Configuration());

  // The source dataflow
  ElementSpecPtr sourceS = conf->addElement(ElementPtr(new TimedPushSource("source", 1)));
  ElementSpecPtr sourcePrintS = conf->addElement(ElementPtr(new Print("AfterSource")));
  ElementSpecPtr transS =       // If the seconds end in 0, produce a
                                // tuple.  Prefix the tuple with a 0 or
                                // 1 based on whether the number of
                                // seconds was odd or even
    conf->addElement(ElementPtr(new PelTransform("trans", "$1 10 % dup $1 2 % ->i32 ifpop ifpoptuple")));
  ElementSpecPtr prefixedPrintS = conf->addElement(ElementPtr(new Print("Prefixed")));
  conf->hookUp(sourceS, 0, sourcePrintS, 0);
  conf->hookUp(sourcePrintS, 0, transS, 0);
  conf->hookUp(transS, 0, prefixedPrintS, 0);


  // The even destination dataflow
  ElementSpecPtr sinkPrintEvenS = conf->addElement(ElementPtr(new Print("EvenSink")));
  ElementSpecPtr sinkEvenS = conf->addElement(ElementPtr(new Discard("discardEven")));
  conf->hookUp(sinkPrintEvenS, 0, sinkEvenS, 0);

  // The odd destination dataflow
  ElementSpecPtr sinkPrintOddS = conf->addElement(ElementPtr(new Print("OddSink")));
  ElementSpecPtr sinkOddS = conf->addElement(ElementPtr(new Discard("discardOdd")));
  conf->hookUp(sinkPrintOddS, 0, sinkOddS, 0);

  // The other destination dataflow
  ElementSpecPtr sinkPrintOtherS = conf->addElement(ElementPtr(new Print("OtherSink")));
  ElementSpecPtr sinkOtherS = conf->addElement(ElementPtr(new Discard("discardOther")));
  conf->hookUp(sinkPrintOtherS, 0, sinkOtherS, 0);

  // The demultiplexer
  boost::shared_ptr< std::vector< ValuePtr > > demuxKeys(new std::vector< ValuePtr >);
  demuxKeys->push_back(ValuePtr(new Val_Int32(0)));
  demuxKeys->push_back(ValuePtr(new Val_Int32(1)));
  ElementSpecPtr demuxS = conf->addElement(ElementPtr(new Demux("demux", demuxKeys)));
  conf->hookUp(prefixedPrintS, 0, demuxS, 0);
  conf->hookUp(demuxS, 0, sinkPrintEvenS, 0);
  conf->hookUp(demuxS, 1, sinkPrintOddS, 0);
  conf->hookUp(demuxS, 2, sinkPrintOtherS, 0);
  
  
  PlumberPtr plumber(new Plumber(conf));
  if (plumber->initialize(plumber) == 0) {
    std::cout << "Correctly initialized static demux.\n";
  } else {
    std::cout << "** Failed to initialize correct spec\n";
    return;
  }

  // Activate the plumber
  plumber->activate();

  // Run the plumber
  eventLoop();
}

int main(int argc, char **argv)
{
  std::cout << "\nDEMULTIPLEX\n";
  eventLoopInitialize();

  testStaticDemux();

  return 0;
}
  

/*
 * End of file 
 */
