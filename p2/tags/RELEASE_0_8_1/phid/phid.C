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
 * DESCRIPTION: Phi daemon
 *
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */
#include <iostream>

#include "plsensor.h"
#include "plumber.h"

#include "csvparser.h"
#include "print.h"
#include "timedPullSink.h"

#include "loop.h"

const char *path="/snort/tcpconns";
const uint16_t port = 12337;
//const char *path="/slicestat";
//const uint16_t port = 3100;

/** Test the Rx part of the Udp element. */
int main(int argc, char **argv)
{
  TELL_INFO << "\nPhi daemon started\n";
  eventLoopInitialize();

  Plumber::ConfigurationPtr conf(new Plumber::Configuration());
  ElementSpecPtr pl = conf->addElement(ElementPtr(new PlSensor("PLSensor", port, path, 30)));
  ElementSpecPtr csv = conf->addElement(ElementPtr(new CSVParser("CSVParser")));
  ElementSpecPtr print = conf->addElement(ElementPtr(new Print("Printer")));
  ElementSpecPtr sink = conf->addElement(ElementPtr(new TimedPullSink("sink", 0)));
  conf->hookUp(pl,0,csv,0);
  conf->hookUp(csv,0,print,0);
  conf->hookUp(print,0, sink, 0);

  // Create the plumber and check it statically
  PlumberPtr plumber(new Plumber(conf));
  if (plumber->initialize(plumber) == 0) {
    TELL_INFO << "Correctly initialized configuration.\n";
  } else {
    TELL_INFO << "** Failed to initialize correct spec\n";
  }

  // Activate the plumber
  plumber->activate();

  // Run the plumber
  eventLoop();
}
  

/*
 * End of file 
 */
