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
 * DESCRIPTION: Tests for scheduling, timed tasks, repeated tasks,
 * queues, etc.
 *
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */
#include <iostream>

#include "tuple.h"
#include "plumber.h"

#include "print.h"
#include "timedPushSource.h"
#include "timedPullSink.h"
#include "pelTransform.h"
#include "slot.h"

/** Test push chain with fast source. */
void testFastSourcePush()
{
  std::cout << "\nCHECK FAST SOURCE PUSH\n";

  PlumberPtr plumber(new Plumber());
  Plumber::DataflowPtr conf = plumber->new_dataflow("test");
  ElementSpecPtr sourceS = conf->addElement(ElementPtr(new TimedPushSource("source", 0.2)));
  ElementSpecPtr sinkS = conf->addElement(ElementPtr(new TimedPullSink("sink", 1)));
  ElementSpecPtr slotS = conf->addElement(ElementPtr(new Slot("slot")));
  ElementSpecPtr sinkPrintS = conf->addElement(ElementPtr(new Print("BeforeSink")));
  ElementSpecPtr sourcePrintS = conf->addElement(ElementPtr(new Print("AfterSource")));
  ElementSpecPtr transS = conf->addElement(ElementPtr(new PelTransform("trans", "$1 10 % ifpoptuple")));

  conf->hookUp(sourceS, 0, sourcePrintS, 0);
  conf->hookUp(sourcePrintS, 0, transS, 0);
  conf->hookUp(transS, 0, slotS, 0);
  conf->hookUp(slotS, 0, sinkPrintS, 0);
  conf->hookUp(sinkPrintS, 0, sinkS, 0);
  
  if (plumber->install(conf) == 0) {
    std::cout << "Correctly initialized fast source push.\n";
  } else {
    std::cout << "** Failed to initialize correct spec\n";
  }

  // Run the plumber
  eventLoop();
}

/** Test push chain with fast sink. */
void testFastSinkPush()
{
  std::cout << "\nCHECK FAST SINK PUSH\n";
  eventLoopInitialize();
  PlumberPtr plumber(new Plumber());
  Plumber::DataflowPtr conf = plumber->new_dataflow("test");

  ElementSpecPtr sourceS = conf->addElement(ElementPtr(new TimedPushSource("source", 1)));
  ElementSpecPtr sinkS = conf->addElement(ElementPtr(new TimedPullSink("sink", .2)));
  ElementSpecPtr slotS = conf->addElement(ElementPtr(new Slot("slot")));
  ElementSpecPtr sinkPrintS = conf->addElement(ElementPtr(new Print("BeforeSink")));
  ElementSpecPtr sourcePrintS = conf->addElement(ElementPtr(new Print("AfterSource")));
  ElementSpecPtr transS = conf->addElement(ElementPtr(new PelTransform("trans", "$1 10 % ifpoptuple")));

  conf->hookUp(sourceS, 0, sourcePrintS, 0);
  conf->hookUp(sourcePrintS, 0, transS, 0);
  conf->hookUp(transS, 0, slotS, 0);
  conf->hookUp(slotS, 0, sinkPrintS, 0);
  conf->hookUp(sinkPrintS, 0, sinkS, 0);
  
  if (plumber->install(conf) == 0) {
    std::cout << "Correctly initialized fast sink push.\n";
  } else {
    std::cout << "** Failed to initialize correct spec\n";
  }

  // Run the plumber
  eventLoop();
}

/** Test push chain with fast sink. */
void testFastSinkPull()
{
  std::cout << "\nCHECK FAST SINK PULL\n";

  PlumberPtr plumber(new Plumber());
  Plumber::DataflowPtr conf = plumber->new_dataflow("test");
  ElementSpecPtr sourceS = conf->addElement(ElementPtr(new TimedPushSource("source", 1)));
  ElementSpecPtr sinkS = conf->addElement(ElementPtr(new TimedPullSink("sink", .2)));
  ElementSpecPtr slotS = conf->addElement(ElementPtr(new Slot("slot")));
  ElementSpecPtr sinkPrintS = conf->addElement(ElementPtr(new Print("BeforeSink")));
  ElementSpecPtr sourcePrintS = conf->addElement(ElementPtr(new Print("AfterSource")));
  ElementSpecPtr transS = conf->addElement(ElementPtr(new PelTransform("trans", "$1 10 % ifpoptuple")));

  conf->hookUp(sourceS, 0, sourcePrintS, 0);
  conf->hookUp(sourcePrintS, 0, slotS, 0);
  conf->hookUp(slotS, 0, transS, 0);
  conf->hookUp(transS, 0, sinkPrintS, 0);
  conf->hookUp(sinkPrintS, 0, sinkS, 0);
  
  if (plumber->install(conf) == 0) {
    std::cout << "Correctly initialized fast sink pull.\n";
  } else {
    std::cout << "** Failed to initialize correct spec\n";
  }

  // Run the plumber
  eventLoop();
}

/** Test push chain with fast sink. */
void testFastSourcePull()
{
  std::cout << "\nCHECK FAST SOURCE PULL\n";

  PlumberPtr plumber(new Plumber());
  Plumber::DataflowPtr conf = plumber->new_dataflow("test");
  ElementSpecPtr sourceS = conf->addElement(ElementPtr(new TimedPushSource("source", .2)));
  ElementSpecPtr sinkS = conf->addElement(ElementPtr(new TimedPullSink("sink", 1)));
  ElementSpecPtr slotS = conf->addElement(ElementPtr(new Slot("slot")));
  ElementSpecPtr sinkPrintS = conf->addElement(ElementPtr(new Print("BeforeSink")));
  ElementSpecPtr sourcePrintS = conf->addElement(ElementPtr(new Print("AfterSource")));
  ElementSpecPtr transS = conf->addElement(ElementPtr(new PelTransform("trans", "$1 10 % ifpoptuple")));

  conf->hookUp(sourceS, 0, sourcePrintS, 0);
  conf->hookUp(sourcePrintS, 0, slotS, 0);
  conf->hookUp(slotS, 0, transS, 0);
  conf->hookUp(transS, 0, sinkPrintS, 0);
  conf->hookUp(sinkPrintS, 0, sinkS, 0);
  
  if (plumber->install(conf) == 0) {
    std::cout << "Correctly initialized fast sink pull.\n";
  } else {
    std::cout << "** Failed to initialize correct spec\n";
  }

  // Run the plumber
  eventLoop();
}

int main(int argc, char **argv)
{
  std::cout << "\nSCHEDULING\n";

  testFastSourcePull();

  return 0;
}
  

/*
 * End of file 
 */
