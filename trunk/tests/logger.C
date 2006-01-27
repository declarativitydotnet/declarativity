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
 * DESCRIPTION: Tests for logger element
 *
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */
#include <iostream>

#include "logger.h"
#include "print.h"

#include "elementSpec.h"
#include "plumber.h"
#include "print.h"
#include "discard.h"
#include "slot.h"


/** Test the Logger. */
void testLogger()
{
  std::cout << "\nCHECK LOGGER\n";

  boost::shared_ptr<Logger> log(new Logger("theLogger"));
  Plumber::ConfigurationPtr conf(new Plumber::Configuration());
  ElementSpecPtr logSpec = conf->addElement(log);
  ElementSpecPtr sinkPrintS = conf->addElement(ElementPtr(new Print("BeforeSink")));
  ElementSpecPtr sinkS = conf->addElement(ElementPtr(new Discard("sink")));
  conf->hookUp(logSpec, 0, sinkPrintS, 0);
  conf->hookUp(sinkPrintS, 0, sinkS, 0);

  PlumberPtr plumber(new Plumber(conf));
  if (plumber->initialize(plumber) == 0) {
    std::cout << "Correctly initialized spec.\n";
  } else {
    std::cout << "** Failed to initialize correct spec\n";
  }

  // Activate the plumber
  plumber->activate();
  plumber->logger(log.get());

  std::cout << "Plumber activated, captain.\n";

  for( int i=0; i<5; i++) {
    log->log( "test class",
	      "test instance",
	      Logger::WARN,
	      i, 
	      "Test message");
  }
}

int main(int argc, char **argv)
{
  std::cout << "\nLOGGER\n";

  testLogger();

  return 0;
}
  

/*
 * End of file 
 */
 
